#pragma once
#include <QWidget>
#include "ui_DashboardPage.h"

class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget* parent = nullptr);
    ~DashboardPage();

    // Updated to match what AppController is calling
    void updateBalance(double balance, const QString& fullName = "Nasir");

private slots:
    void onShowBalanceClicked();
    void onAddFundsConfirmClicked();
    void onSendMoneyClicked();
    void onPayNowClicked();
    void onRechargeClicked();
    void setupSidebarNavigation();
    void refreshBalance();

private:
    Ui::DashboardPageClass ui;
    double currentBalance = 12450.75;
    bool balanceVisible = false;

    void loadDummyData();
    void showSuccessMessage(const QString& title, const QString& message);
    void showErrorMessage(const QString& title, const QString& message);
};