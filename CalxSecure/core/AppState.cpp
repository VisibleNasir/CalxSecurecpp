#include "AppState.h"

// ================= SINGLETON =================
AppState& AppState::instance() {
    static AppState instance;
    return instance;
}

// ================= TOKEN =================
QString AppState::token() const {
    return m_token;
}

void AppState::setToken(const QString& newToken) {
    if (m_token != newToken) {
        m_token = newToken;
        emit authChanged();
    }
}

// ================= USER =================
QString AppState::userId() const {
    return m_userId;
}

void AppState::setUserId(const QString& newUserId) {
    if (m_userId != newUserId) {
        m_userId = newUserId;
        emit authChanged();
    }
}

QString AppState::fullName() const {
    return m_fullName;
}

QString AppState::email() const {
    return m_email;
}

// ================= BALANCE =================
double AppState::balance() const {
    return m_balance;
}

void AppState::setBalance(double newBalance) {
    if (!qFuzzyCompare(m_balance, newBalance)) {
        m_balance = newBalance;
        emit balanceChanged();
    }
}

// ================= SESSION =================
void AppState::setSession(const SessionData& data) {
    m_userId = data.userId;
    m_fullName = data.fullName;
    m_email = data.email;

    setBalance(data.balance); // ensures signal emission

    emit authChanged();
}