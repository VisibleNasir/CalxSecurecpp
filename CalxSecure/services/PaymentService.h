#pragma once

#include <QObject>
#include <QString>

class PaymentService : public QObject
{
    Q_OBJECT

public:
    static PaymentService& instance();

    QString createAddFunds(int userId, double amount);
    bool completeTransaction(const QString& txId);
    double getUserBalance(int userId);

signals:
    void paymentCompleted(bool success, double newBalance);

private:
    explicit PaymentService(QObject* parent = nullptr);
    Q_DISABLE_COPY(PaymentService)
};