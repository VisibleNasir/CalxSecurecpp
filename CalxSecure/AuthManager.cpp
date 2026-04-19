// ================================================
// STEP 3: AUTH SYSTEM Implementation
// ================================================
// File: auth/AuthManager.cpp

#include "AuthManager.h"
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QDebug>

AuthManager::AuthManager(QObject* parent)
    : QObject(parent),
    m_db(DatabaseManager::instance())
{
}

AuthManager::~AuthManager() = default;

QByteArray AuthManager::generateSalt(int length)
{
    QByteArray salt(length, 0);
    QRandomGenerator::global()->fillRange(reinterpret_cast<quint32*>(salt.data()), length / 4);
    return salt.toHex(); // Store as hex for readability in DB
}

QString AuthManager::generateSessionToken(int length)
{
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    QString token;
    token.reserve(length);

    for (int i = 0; i < length; ++i) {
        int idx = QRandomGenerator::global()->bounded(chars.length());
        token.append(chars.at(idx));
    }
    return token;
}

QByteArray AuthManager::hashPassword(const QString& password, const QByteArray& salt)
{
    QByteArray data = password.toUtf8() + salt;
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);

    // Simple PBKDF-like stretching (10 iterations) - better than plain SHA-256
    for (int i = 0; i < 10; ++i) {
        hash = QCryptographicHash::hash(hash + salt, QCryptographicHash::Sha256);
    }
    return hash.toHex();
}

bool AuthManager::signup(const QString& username, const QString& email,
    const QString& password, const QString& fullName,
    const QString& phone, const QString& role)
{
    if (password.length() < 8) {
        emit signupFailed("Password must be at least 8 characters");
        return false;
    }

    // Input sanitization
    if (username.isEmpty() || email.isEmpty() || role.isEmpty()) {
        emit signupFailed("All fields are required");
        return false;
    }

    QByteArray salt = generateSalt();
    QByteArray hashedPassword = hashPassword(password, salt);

    // Check if user already exists
    QSqlQuery check = m_db.executeQuery(
        "SELECT id FROM users WHERE email = ? OR username = ?",
        { email, username });

    if (check.next()) {
        emit signupFailed("User with this email or username already exists");
        return false;
    }

    // Insert user
    QSqlQuery query = m_db.executeQuery(
        "INSERT INTO users (username, email, password_hash, role, full_name, phone) "
        "VALUES (?, ?, ?, ?, ?, ?) RETURNING id",
        { username, email, QString(hashedPassword), role, fullName, phone });

    if (!query.next()) {
        emit signupFailed("Failed to create account");
        return false;
    }

    int userId = query.value(0).toInt();

    // Role-specific setup
    if (role == "merchant") {
        createMerchantProfile(userId, fullName + "'s Business");
    }
    createRewardEntry(userId);

    emit signupSuccess();
    qInfo() << "New" << role << "account created:" << username;
    return true;
}

bool AuthManager::login(const QString& email, const QString& password, const QString& roleHint)
{
    QSqlQuery query = m_db.executeQuery(
        "SELECT id, username, full_name, role, password_hash FROM users "
        "WHERE email = ? AND is_active = true",
        { email });

    if (!query.next()) {
        emit loginFailed("Invalid email or password");
        return false;
    }

    int userId = query.value(0).toInt();
    QString dbRole = query.value(3).toString();
    QString storedHash = query.value(4).toString();

    // Role hint validation (optional security layer)
    if (!roleHint.isEmpty() && roleHint != dbRole) {
        emit loginFailed("Access denied for this role");
        return false;
    }

    // In real app, retrieve salt from a separate column or derive it.
    // For simplicity here we re-hash with a stored salt column (add salt column in schema if needed).
    // Current implementation uses fixed stretching - production should store salt per user.

    QByteArray inputHash = hashPassword(password, QByteArray::fromHex("dummy_salt_for_demo")); // Replace with real salt retrieval

    if (QString(inputHash) != storedHash) {
        emit loginFailed("Invalid email or password");
        return false;
    }

    // Generate session
    QString token = generateSessionToken(64);
    QDateTime expires = QDateTime::currentDateTime();

    //QDateTime expires = QDateTime::currentDateTime().addHours(24);

    if (!storeSessionToken(userId, token)) {
        emit loginFailed("Session creation failed");
        return false;
    }

    // Populate session
    m_currentSession.userId = userId;
    m_currentSession.username = query.value(1).toString();
    m_currentSession.fullName = query.value(2).toString();
    m_currentSession.email = email;
    m_currentSession.role = dbRole;
    m_currentSession.token = token;
    m_currentSession.expiresAt = expires;
    m_currentSession.isAuthenticated = true;

    // Update last login
    m_db.executeQuery("UPDATE users SET last_login = NOW() WHERE id = ?", { userId });

    emit loginSuccess(m_currentSession);
    emit authStateChanged(true);

    qInfo() << "✅ Login successful for" << dbRole << ":" << email;
    return true;
}

bool AuthManager::logout()
{
    if (m_currentSession.userId > 0) {
        m_db.executeQuery("DELETE FROM sessions WHERE token = ?", { m_currentSession.token });
    }

    m_currentSession = UserSession(); // reset
    emit authStateChanged(false);
    return true;
}

bool AuthManager::isAuthenticated() const
{
    return m_currentSession.isAuthenticated && m_currentSession.expiresAt > QDateTime::currentDateTime();
}

const UserSession& AuthManager::currentSession() const
{
    return m_currentSession;
}

bool AuthManager::validateSession()
{
    if (!isAuthenticated()) return false;

    QSqlQuery q = m_db.executeQuery(
        "SELECT 1 FROM sessions WHERE token = ? AND expires_at > NOW()",
        { m_currentSession.token });

    return q.next();
}

bool AuthManager::isAdmin() const { return m_currentSession.role == "admin"; }
bool AuthManager::isMerchant() const { return m_currentSession.role == "merchant"; }
bool AuthManager::isRegularUser() const { return m_currentSession.role == "user"; }

bool AuthManager::storeSessionToken(int userId, const QString& token)
{
    //QDateTime expires = QDateTime::currentDateTime().addHours(24);
    QDateTime expires = QDateTime::currentDateTime();
    auto q = m_db.executeQuery(
        "INSERT INTO sessions (user_id, token, expires_at) VALUES (?, ?, ?)",
        { userId, token, expires });
    return q.numRowsAffected() > 0;
}

bool AuthManager::createMerchantProfile(int userId, const QString& businessName)
{
    return m_db.executeQuery(
        "INSERT INTO merchants (user_id, business_name) VALUES (?, ?)",
        { userId, businessName }).numRowsAffected() > 0;
}

bool AuthManager::createRewardEntry(int userId)
{
    QString referral = "REF" + QString::number(userId).rightJustified(6, '0');
    return m_db.executeQuery(
        "INSERT INTO rewards (user_id, referral_code) VALUES (?, ?)",
        { userId, referral }).numRowsAffected() > 0;
}