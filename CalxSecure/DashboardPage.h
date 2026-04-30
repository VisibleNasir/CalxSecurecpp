#pragma once

#include <QWidget>
#include <QListWidget>
#include "ui_DashboardPage.h"
#include "services/PaymentService.h"

class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    DashboardPage(QWidget* parent = nullptr);
    ~DashboardPage();

    void loadDummyTransactions();
    void updateBalance(double balance, const QString& fullName);

private slots:
    void onShowBalanceClicked();
    void onAddFundsClicked();
    void onSendMoneyClicked();
    void refreshBalance();
    void simulatePayment(const QString& txId);
    void onPaymentCompleted(bool success, double balance);

private:
    Ui::DashboardPageClass ui;

    double currentBalance;
};