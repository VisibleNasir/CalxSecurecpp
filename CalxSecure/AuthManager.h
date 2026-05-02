#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <QObject>
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QDateTime>

struct UserSession {
    int userId = 0;
    QString username;
    QString fullName;
    QString email;
    QString role;
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

    bool signup(const QString& username, const QString& email, const QString& password,
        const QString& fullName, const QString& phone, const QString& role);

    bool login(const QString& email, const QString& password, const QString& roleHint = "");

    const UserSession& currentSession() const;
    bool logout();

signals:
    void signupSuccess();
    void signupFailed(const QString& message);
    void loginSuccess(const UserSession& session);
    void loginFailed(const QString& message);
    void authStateChanged(bool authenticated);

private:
    QByteArray generateSalt(int length = 32);
    QByteArray derivePBKDF2(const QString& password, const QByteArray& salt, int iterations = 600000);
    QString hashForStorage(const QString& password, const QByteArray& salt);
    QString generateSessionToken(int length);
    bool storeSessionToken(int userId, const QString& token, const QDateTime& expires);
    DatabaseManager& m_db;
    UserSession m_currentSession;
};

#endif // AUTHMANAGER_H