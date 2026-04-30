#include "PaymentService.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

// ================= SINGLETON =================
PaymentService& PaymentService::instance()
{
    static PaymentService instance;
    return instance;
}

PaymentService::PaymentService(QObject* parent)
    : QObject(parent)
{
}

// ================= CREATE TRANSACTION =================
QString PaymentService::createAddFunds(int userId, double amount)
{
    QSqlQuery query;

    query.prepare(R"(
        INSERT INTO transactions (from_user_id, amount, type, status)
        VALUES (:userId, :amount, 'add_funds', 'pending')
        RETURNING tx_id
    )");

    query.bindValue(":userId", userId);
    query.bindValue(":amount", amount);

    if (!query.exec()) {
        qDebug() << "Create Tx Error:" << query.lastError().text();
        return "";
    }

    if (query.next()) {
        return query.value(0).toString();
    }

    return "";
}

// ================= COMPLETE TRANSACTION =================
bool PaymentService::completeTransaction(const QString& txId)
{
    QSqlQuery query;

    // 1. Get transaction
    query.prepare("SELECT amount, from_user_id, status FROM transactions WHERE tx_id = :txId");
    query.bindValue(":txId", txId);

    if (!query.exec() || !query.next()) {
        qDebug() << "Tx fetch failed";
        emit paymentCompleted(false, 0);
        return false;
    }

    double amount = query.value(0).toDouble();
    int userId = query.value(1).toInt();
    QString status = query.value(2).toString();

    if (status == "completed") {
        qDebug() << "Already processed";
        emit paymentCompleted(false, 0);
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    // 2. Update transaction
    QSqlQuery updateTx;
    updateTx.prepare("UPDATE transactions SET status='completed', completed_at=NOW() WHERE tx_id=:txId");
    updateTx.bindValue(":txId", txId);

    if (!updateTx.exec()) {
        db.rollback();
        emit paymentCompleted(false, 0);
        return false;
    }

    // 3. Update balance
    QSqlQuery updateBalance;
    updateBalance.prepare("UPDATE users SET balance = balance + :amount WHERE id = :userId");
    updateBalance.bindValue(":amount", amount);
    updateBalance.bindValue(":userId", userId);

    if (!updateBalance.exec()) {
        db.rollback();
        emit paymentCompleted(false, 0);
        return false;
    }

    db.commit();

    double newBalance = getUserBalance(userId);

    emit paymentCompleted(true, newBalance);
    return true;
}

// ================= GET BALANCE =================
double PaymentService::getUserBalance(int userId)
{
    QSqlQuery query;
    query.prepare("SELECT balance FROM users WHERE id = :id");
    query.bindValue(":id", userId);

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}