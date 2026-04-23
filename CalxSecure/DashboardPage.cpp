#include "DashboardPage.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#include <QSqlQuery>
#include <QSqlError>

#include "core/AppState.h"
#include "services/PaymentService.h"

DashboardPage::DashboardPage(QWidget* parent)
    : QWidget(parent), currentBalance(0.0)
{
    ui.setupUi(this);

    ui.lblBalance->setText("₹ ****");

    // Connections
    connect(ui.btnShowBalance, &QPushButton::clicked,
        this, &DashboardPage::onShowBalanceClicked);

    connect(ui.btnAddFunds, &QPushButton::clicked,
        this, &DashboardPage::onAddFundsClicked);

    connect(ui.btnSendMoney, &QPushButton::clicked,
        this, &DashboardPage::onSendMoneyClicked);

    // Payment completion signal
    connect(&PaymentService::instance(), &PaymentService::paymentCompleted,
        this, &DashboardPage::onPaymentCompleted);

    // Auto refresh
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &DashboardPage::refreshBalance);
    timer->start(3000);
}

DashboardPage::~DashboardPage()
{
}

// ================= BALANCE =================

void DashboardPage::refreshBalance()
{
    int userId = AppState::instance().userId().toInt();  // ✅ FIXED

    QSqlQuery query;
    query.prepare("SELECT balance FROM users WHERE id = :id");
    query.bindValue(":id", userId);

    if (query.exec() && query.next()) {
        double balance = query.value(0).toDouble();

        currentBalance = balance;

        // sync with AppState
        AppState::instance().setBalance(balance);
    }
}

void DashboardPage::updateBalance(double balance, const QString& fullName)
{
    currentBalance = balance;

    ui.lblUserName->setText(fullName);

    // keep hidden
    ui.lblBalance->setText("₹ ****");
}

// ================= BALANCE VISIBILITY =================

void DashboardPage::onShowBalanceClicked()
{
    bool ok;
    QString pin = QInputDialog::getText(this, "Enter PIN",
        "4-digit PIN:",
        QLineEdit::Password, "", &ok);

    if (!ok) return;

    if (pin.length() != 4 || !pin.toInt()) {
        QMessageBox::warning(this, "Error", "Invalid PIN format");
        return;
    }

    if (pin != "1234") {
        QMessageBox::critical(this, "Access Denied", "Incorrect PIN");
        return;
    }

    ui.lblBalance->setText(QString("₹ %1").arg(currentBalance, 0, 'f', 2));

    QTimer::singleShot(10000, this, [this]() {
        ui.lblBalance->setText("₹ ****");
        });
}

// ================= SEND MONEY =================

void DashboardPage::onSendMoneyClicked()
{
    QMessageBox::information(this, "Send Money", "Coming soon.");
}

// ================= ADD FUNDS =================

void DashboardPage::onAddFundsClicked()
{
    double amount = ui.amountInput->value();

    if (amount <= 0) {
        QMessageBox::warning(this, "Error", "Enter valid amount");
        return;
    }

    int userId = AppState::instance().userId().toInt(); // ✅ FIXED

    QString txId = PaymentService::instance().createAddFunds(userId, amount);

    if (txId.isEmpty()) {
        QMessageBox::critical(this, "Error", "Transaction failed");
        return;
    }

    // Loading UI
    ui.btnAddFunds->setText("Processing...");
    ui.btnAddFunds->setEnabled(false);

    simulatePayment(txId);
}

// ================= SIMULATION =================

void DashboardPage::simulatePayment(const QString& txId)
{
    QTimer::singleShot(2500, this, [this, txId]() {
        bool success = PaymentService::instance().completeTransaction(txId);

        double balance = PaymentService::instance().getUserBalance(
            AppState::instance().userId().toInt()
        );

        onPaymentCompleted(success, balance);
        });
}

// ================= CALLBACK =================

void DashboardPage::onPaymentCompleted(bool success, double balance)
{
    ui.btnAddFunds->setEnabled(true);
    ui.btnAddFunds->setText("Add Funds");

    if (success) {
        AppState::instance().setBalance(balance);

        updateBalance(balance, AppState::instance().fullName()); // ✅ FIXED

        QMessageBox::information(this, "Success", "Funds added successfully");
    }
    else {
        QMessageBox::warning(this, "Failed", "Payment failed");
    }
}

// ================= DUMMY =================

void DashboardPage::loadDummyTransactions()
{
    ui.transactionsList->clear();

    QStringList data = {
        "Completed  | ₹500   | Rahul",
        "Pending    | ₹1200  | Amazon",
        "Failed     | ₹300   | Payment Error"
    };

    for (const QString& item : data) {
        ui.transactionsList->addItem(item);
    }
}