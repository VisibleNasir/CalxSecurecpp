#pragma once
#include <QObject>
#include <QString>

// ================= SESSION STRUCT =================
struct SessionData {
    QString userId;
    QString fullName;
    QString email;
    QString phone;
    double balance = 0.0;
};

// ================= APP STATE =================
class AppState : public QObject {
    Q_OBJECT

public:
    static AppState& instance();

    // ===== AUTH TOKEN =====
    QString token() const;
    void setToken(const QString& newToken);

    // ===== USER =====
    QString userId() const;
    void setUserId(const QString& newUserId);

    QString fullName() const;   // ✅ FIXED (was missing)
    QString email() const;      // ✅ FIXED (was missing)

    // ===== BALANCE =====
    double balance() const;
    void setBalance(double newBalance);
    QString phone() const;           // ← Add this
    void setPhone(const QString& newPhone);  // ← Add this
    // ===== SESSION =====
    bool isLoggedIn() const { return !m_userId.isEmpty(); }
    void setSession(const SessionData& data);

signals:
    void authChanged();
    void balanceChanged();

private:
    AppState() = default;
    ~AppState() = default;
    Q_DISABLE_COPY(AppState)

        QString m_token;
    QString m_userId;
    QString m_fullName;
    QString m_phone;
    QString m_email;
    double m_balance = 0.0;
};