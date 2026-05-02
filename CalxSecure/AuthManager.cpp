#include "AuthManager.h"
#include <QCryptographicHash>
#include <QPasswordDigestor>
#include <QRandomGenerator>
#include <QDebug>

AuthManager::AuthManager(QObject* parent)
    : QObject(parent), m_db(DatabaseManager::instance())
{
}

AuthManager::~AuthManager() = default;

// ==================== PBKDF2 HASHING ====================
QByteArray AuthManager::generateSalt(int length)
{
    QByteArray salt(length, 0);
    QRandomGenerator::global()->fillRange(reinterpret_cast<quint32*>(salt.data()), length / 4);
    return salt;
}

QByteArray AuthManager::derivePBKDF2(const QString& password, const QByteArray& salt, int iterations)
{
    constexpr int KEY_LENGTH = 64;
    return QPasswordDigestor::deriveKeyPbkdf2(
        QCryptographicHash::Sha256,
        password.toUtf8(),
        salt,
        iterations,
        KEY_LENGTH
    );
}

QString AuthManager::hashForStorage(const QString& password, const QByteArray& salt)
{
    QByteArray derived = derivePBKDF2(password, salt, 60000);
    return QString::fromLatin1(derived.toHex());
}

// ==================== SESSION HELPERS ====================
QString AuthManager::generateSessionToken(int length)
{
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    QString token;
    for (int i = 0; i < length; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.length());
        token.append(chars[index]);
    }
    return token;
}

bool AuthManager::storeSessionToken(int userId, const QString& token, const QDateTime& expires)
{
    QSqlQuery q = m_db.executeQuery(
        "INSERT INTO sessions (user_id, token, expires_at) VALUES (?, ?, ?)",
        { userId, token, expires });
    return q.numRowsAffected() > 0;
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
    if (username.isEmpty() || email.isEmpty()) {
        emit signupFailed("Username and Email are required");
        return false;
    }

    QByteArray salt = generateSalt(32);
    QString hashedPassword = hashForStorage(password, salt);
    QString saltHex = QString::fromLatin1(salt.toHex()); // Force String to avoid DB byte mangle

    QSqlQuery check = m_db.executeQuery(
        "SELECT 1 FROM users WHERE email = ? OR username = ?",
        { email, username });

    if (check.next()) {
        emit signupFailed("User with this email or username already exists");
        return false;
    }

    QString sql = R"(
        INSERT INTO users (username, email, password_hash, salt, role, full_name, phone, balance)
        VALUES (?, ?, ?, ?, ?, ?, ?, 1000.00)
        RETURNING id
    )";

    QSqlQuery query = m_db.executeQuery(sql, {
        username, email, hashedPassword, saltHex, role, fullName, phone
        });

    if (!query.isActive() || query.lastError().isValid()) {
        emit signupFailed("Database error: " + query.lastError().text());
        return false;
    }

    if (!query.next()) {
        emit signupFailed("Failed to create account");
        return false;
    }

    qInfo() << "✅ New user created:" << username << "ID:" << query.value(0).toInt();
    emit signupSuccess();
    return true;
}

// ==================== LOGIN ====================
bool AuthManager::login(const QString& email, const QString& password, const QString& roleHint)
{
    QSqlQuery query = m_db.executeQuery(R"(
        SELECT id, username, full_name, role, password_hash, salt, COALESCE(balance, 0) as balance
        FROM users WHERE email = ? AND is_active = true
    )", { email });

    if (!query.next()) {
        emit loginFailed("Invalid email or password");
        return false;
    }

    int userId = query.value(0).toInt();
    QString dbRole = query.value(3).toString();
    QString storedHashHex = query.value(4).toString();
    QString saltString = query.value(5).toString(); // Safely extract string

    // Reconstruct raw salt
    QByteArray salt = QByteArray::fromHex(saltString.toLatin1());

    if (!roleHint.isEmpty() && roleHint.toLower() != dbRole.toLower()) {
        emit loginFailed("Access denied for this role");
        return false;
    }

    QByteArray derivedKey = derivePBKDF2(password, salt, 60000);
    QString inputHashHex = QString::fromLatin1(derivedKey.toHex());

    if (inputHashHex != storedHashHex) {
        emit loginFailed("Invalid email or password");
        return false;
    }

    // Auth succeeded, setup session...
    QString token = generateSessionToken(64);
    QDateTime expires = QDateTime::currentDateTime().addSecs(24 * 60 * 60);

    if (!storeSessionToken(userId, token, expires)) {
        emit loginFailed("Session creation failed");
        return false;
    }

    m_currentSession.userId = userId;
    m_currentSession.username = query.value(1).toString();
    m_currentSession.fullName = query.value(2).toString();
    m_currentSession.email = email;
    m_currentSession.role = dbRole;
    m_currentSession.token = token;
    m_currentSession.expiresAt = expires;
    m_currentSession.balance = query.value("balance").toDouble();
    m_currentSession.isAuthenticated = true;

    m_db.executeQuery("UPDATE users SET last_login = NOW() WHERE id = ?", { userId });

    emit loginSuccess(m_currentSession);
    emit authStateChanged(true);
    qInfo() << "✅ Login successful for:" << email;
    return true;
}

const UserSession& AuthManager::currentSession() const
{
    return m_currentSession;
}

bool AuthManager::logout()
{
    m_currentSession = UserSession();
    emit authStateChanged(false);
    return true;
}