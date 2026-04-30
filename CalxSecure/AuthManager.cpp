#include "AuthManager.h"
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QDebug>

AuthManager::AuthManager(QObject* parent)
    : QObject(parent),
    m_db(DatabaseManager::instance())
{
}

AuthManager::~AuthManager() = default;

// ==================== PASSWORD HASHING ====================

QByteArray AuthManager::generateSalt(int length)
{
    QByteArray salt(length, 0);
    QRandomGenerator::global()->fillRange(reinterpret_cast<quint32*>(salt.data()), length / 4);
    return salt.toHex();
}

QString AuthManager::hashPassword(const QString& password, const QByteArray& salt)
{
    QByteArray data = password.toUtf8() + salt;
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);

    // Stretch 10 times (simple but better than plain SHA256)
    for (int i = 0; i < 10; ++i) {
        hash = QCryptographicHash::hash(hash + salt, QCryptographicHash::Sha256);
    }
    return QString::fromLatin1(hash.toHex());
}

// ==================== SIGNUP ====================

bool AuthManager::signup(const QString& username, const QString& email,
    const QString& password, const QString& fullName,
    const QString& phone, const QString& role)
{
    if (password.length() < 8) {
        emit signupFailed("Password must be at least 8 characters");
        return false;
    }

    if (username.isEmpty() || email.isEmpty() || role.isEmpty()) {
        emit signupFailed("All fields are required");
        return false;
    }

    // Generate salt + hash
    QByteArray salt = generateSalt(16);
    QString hashedPassword = hashPassword(password, salt);

    // Better existence check
    QSqlQuery check = m_db.executeQuery(
        "SELECT 1 FROM users WHERE email = ? OR username = ?",
        { email, username });

    if (check.next()) {
        emit signupFailed("User with this email or username already exists");
        return false;
    }

    // === MAIN INSERT with proper error handling ===
    QString sql = R"(
        INSERT INTO users (username, email, password_hash, salt, role, full_name, phone)
        VALUES (?, ?, ?, ?, ?, ?, ?)
        RETURNING id
    )";

    QSqlQuery query = m_db.executeQuery(sql, {
        username,
        email,
        hashedPassword,
        QString::fromLatin1(salt),   // ← store salt!
        role,
        fullName,
        phone
        });

    if (!query.isActive() || query.lastError().isValid()) {
        QString err = query.lastError().text();
        qCritical() << "Signup INSERT failed:" << err;
        qDebug() << "SQL:" << sql;
        emit signupFailed("Database error: " + err);
        return false;
    }

    if (!query.next()) {
        qCritical() << "INSERT succeeded but no row returned (RETURNING failed)";
        emit signupFailed("Failed to create account");
        return false;
    }

    int userId = query.value(0).toInt();

    // Role-specific setup
    if (role.toLower() == "merchant") {
        createMerchantProfile(userId, fullName + "'s Business");
    }

    createRewardEntry(userId);

    qInfo() << "✅ New" << role << "account created:" << username << "(ID:" << userId << ")";
    emit signupSuccess();
    return true;
}

// ==================== LOGIN ====================

bool AuthManager::login(const QString& email, const QString& password, const QString& roleHint)
{
    // Fetch user + salt
    QSqlQuery query = m_db.executeQuery(R"(
        SELECT id, username, full_name, role, password_hash, salt 
        FROM users 
        WHERE email = ? AND is_active = true
    )", { email });

    if (!query.next()) {
        emit loginFailed("Invalid email or password");
        return false;
    }

    int userId = query.value(0).toInt();
    QString dbRole = query.value(3).toString();
    QString storedHash = query.value(4).toString();
    QByteArray salt = query.value(5).toByteArray();   // ← crucial

    if (!roleHint.isEmpty() && roleHint.toLower() != dbRole.toLower()) {
        emit loginFailed("Access denied for this role");
        return false;
    }

    // Re-hash input password with stored salt
    QString inputHash = hashPassword(password, salt);

    if (inputHash != storedHash) {
        emit loginFailed("Invalid email or password");
        return false;
    }

    // Create session
    QString token = generateSessionToken(64);
    QDateTime expires = QDateTime::currentDateTime().addSecs(24 * 60 * 60); // 24 hours

    if (!storeSessionToken(userId, token, expires)) {
        emit loginFailed("Session creation failed");
        return false;
    }

    // Fill current session
    m_currentSession.userId = userId;
    m_currentSession.username = query.value(1).toString();
    m_currentSession.fullName = query.value(2).toString();
    m_currentSession.email = email;
    m_currentSession.role = dbRole;
    m_currentSession.token = token;
    m_currentSession.expiresAt = expires;
    m_currentSession.isAuthenticated = true;
    m_currentSession.balance = query.value("balance").toDouble();

    // Update last login
    m_db.executeQuery("UPDATE users SET last_login = NOW() WHERE id = ?", { userId });

    emit loginSuccess(m_currentSession);
    emit authStateChanged(true);
    qInfo() << "✅ Login successful for" << dbRole << ":" << email;

    return true;
}

// ==================== storeSessionToken (small fix) ====================

bool AuthManager::storeSessionToken(int userId, const QString& token, const QDateTime& expires)
{
    auto q = m_db.executeQuery(
        "INSERT INTO sessions (user_id, token, expires_at) VALUES (?, ?, ?)",
        { userId, token, expires });

    return q.numRowsAffected() > 0;
}
// ==================== SESSION ====================

const UserSession& AuthManager::currentSession() const
{
    return m_currentSession;
}

// ==================== TOKEN GENERATION ====================

QString AuthManager::generateSessionToken(int length)
{
    const QString chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789";

    QString token;
    for (int i = 0; i < length; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.length());
        token.append(chars[index]);
    }
    return token;
}

// ==================== ROLE HELPERS ====================

bool AuthManager::createMerchantProfile(int userId, const QString& businessName)
{
    auto q = m_db.executeQuery(
        "INSERT INTO merchants (user_id, business_name) VALUES (?, ?)",
        { userId, businessName });

    return q.numRowsAffected() > 0;
}

bool AuthManager::createRewardEntry(int userId)
{
    auto q = m_db.executeQuery(
        "INSERT INTO rewards (user_id, points) VALUES (?, 0)",
        { userId });

    return q.numRowsAffected() > 0;
}
bool AuthManager::isAuthenticated() const
{
    return m_currentSession.isAuthenticated;
}

bool AuthManager::logout()
{
    m_currentSession = UserSession();
    emit authStateChanged(false);
    return true;
}

bool AuthManager::isAdmin() const
{
    return m_currentSession.role.toLower() == "admin";
}

bool AuthManager::isMerchant() const
{
    return m_currentSession.role.toLower() == "merchant";
}

bool AuthManager::isRegularUser() const
{
    return m_currentSession.role.toLower() == "user";
}