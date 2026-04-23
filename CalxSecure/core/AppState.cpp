#include "AppState.h"

AppState& AppState::instance() {
    static AppState instance;
    return instance;
}

QString AppState::token() const { return m_token; }
void AppState::setToken(const QString& newToken) {
    if (m_token != newToken) {
        m_token = newToken;
        emit authChanged();
    }
}

QString AppState::userId() const { return m_userId; }
void AppState::setUserId(const QString& newUserId) {
    m_userId = newUserId;
}

double AppState::balance() const { return m_balance; }
void AppState::setBalance(double newBalance) {
    if (m_balance != newBalance) {
        m_balance = newBalance;
        emit balanceChanged();
    }
}