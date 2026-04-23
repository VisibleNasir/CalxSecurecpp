#pragma once
#include <QObject>
#include <QString>

class AppState : public QObject {
    Q_OBJECT
public:
    static AppState& instance();

    QString token() const;
    void setToken(const QString& newToken);

    QString userId() const;
    void setUserId(const QString& newUserId);

    double balance() const;
    void setBalance(double newBalance);

signals:
    void authChanged();
    void balanceChanged();

private:
    // Singleton private constructor
    AppState() = default;
    ~AppState() = default;
    Q_DISABLE_COPY(AppState)

    QString m_token;
    QString m_userId;
    double m_balance = 0.0;
};