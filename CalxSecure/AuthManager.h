// ================================================
// STEP 3: AUTH SYSTEM (Production-Grade)
// ================================================
// File: auth/AuthManager.h

#pragma once
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QDateTime>
#include "DatabaseManager.h"

struct UserSession {
    int userId = -1;
    QString username;
    QString fullName;
    QString email;
    QString role;           // "user", "merchant", "admin"
    QString token;
    QDateTime expiresAt;
    double balance = 0.0;
    bool isAuthenticated = false;
};

class AuthManager : public QObject
{
    Q_OBJECT

public:
    explicit AuthManager(QObject* parent = nullptr);
    ~AuthManager();

    // Core Auth Methods
    bool signup(const QString& username, const QString& email, const QString& password,
        const QString& fullName, const QString& phone, const QString& role);

    bool login(const QString& email, const QString& password, const QString& roleHint = "");

    bool logout();

    // Session & Validation
    bool isAuthenticated() const;
    const UserSession& currentSession() const;
    bool validateSession();

    // Role-specific helpers
    bool isAdmin() const;
    bool isMerchant() const;
    bool isRegularUser() const;

    // Password & Token utilities (public for testing)
    static QByteArray hashPassword(const QString& password, const QByteArray& salt);
    static QByteArray generateSalt(int length = 32);
    static QString generateSessionToken(int length = 64);

signals:
    void authStateChanged(bool authenticated);
    void loginSuccess(const UserSession& session);
    void loginFailed(const QString& message);
    void signupSuccess();
    void signupFailed(const QString& message);

private:
    UserSession m_currentSession;
    DatabaseManager& m_db;

    // Internal helpers
    QByteArray getStoredSaltForUser(int userId);
    bool createMerchantProfile(int userId, const QString& businessName = "Default Business");
    bool createRewardEntry(int userId);
    bool storeSessionToken(int userId, const QString& token);
};