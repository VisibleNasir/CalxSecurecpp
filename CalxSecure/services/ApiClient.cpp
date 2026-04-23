#include "ApiClient.h"
#include "../core/AppState.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>

ApiClient& ApiClient::instance() {
    static ApiClient instance;
    return instance;
}

ApiClient::ApiClient(QObject* parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this)) {
    // Replace with your actual backend URL or read from config
    m_baseUrl = "http://localhost:8080/api"; 
}

QNetworkRequest ApiClient::createRequest(const QString& endpoint) {
    QNetworkRequest request(QUrl(m_baseUrl + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Automatically attach token from global state
    QString token = AppState::instance().token();
    if (!token.isEmpty()) {
        request.setRawHeader("Authorization", "Bearer " + token.toUtf8());
    }
    return request;
}

QNetworkReply* ApiClient::post(const QString& endpoint, const QJsonObject& body) {
    QNetworkRequest request = createRequest(endpoint);
    QJsonDocument doc(body);
    QByteArray data = doc.toJson();

    // Caller takes ownership and must call deleteLater() on the reply
    return m_networkManager->post(request, data);
}

QNetworkReply* ApiClient::get(const QString& endpoint) {
    QNetworkRequest request = createRequest(endpoint);
    
    // Caller takes ownership and must call deleteLater() on the reply
    return m_networkManager->get(request);
}