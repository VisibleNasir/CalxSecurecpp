#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QString>

class ApiClient : public QObject {
    Q_OBJECT
public:
    static ApiClient& instance(); // Make it a Singleton for easy global access

    QNetworkReply* post(const QString& endpoint, const QJsonObject& body);
    QNetworkReply* get(const QString& endpoint);

private:
    explicit ApiClient(QObject* parent = nullptr);
    ~ApiClient() = default;
    Q_DISABLE_COPY(ApiClient)

        QNetworkAccessManager* m_networkManager;
    QString m_baseUrl;
    QNetworkRequest createRequest(const QString& endpoint);
};