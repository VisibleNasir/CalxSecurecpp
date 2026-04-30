#include "DashboardPage.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#include <QRandomGenerator>
DashboardPage::DashboardPage(QWidget* parent) : QWidget(parent)
{
	ui.setupUi(this);
	updateBalance(currentBalance, "Nasir");
	loadDummyData();
	setupSidebarNavigation();
	// Example: connect logout button
	connect(ui.btnLogout, &QPushButton::clicked, this, &DashboardPage::onLogoutClicked);
	// Button Connections
	connect(ui.btnShowBalance, &QPushButton::clicked, this, &DashboardPage::onShowBalanceClicked);
	connect(ui.btnAddFundsConfirm, &QPushButton::clicked, this, &DashboardPage::onAddFundsConfirmClicked);
	connect(ui.btnPay, &QPushButton::clicked, this, &DashboardPage::onPayNowClicked);
	connect(ui.btnRecharge, &QPushButton::clicked, this, &DashboardPage::onRechargeClicked);
	connect(ui.btnSendMoney, &QPushButton::clicked, this, &DashboardPage::onSendMoneyClicked);
	connect(ui.btnAddFunds, &QPushButton::clicked, this, [this] {
		ui.stackedWidget->setCurrentWidget(ui.Transfer);
	});
	// Auto refresh
	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &DashboardPage::refreshBalance);
	timer->start(10000);
}
DashboardPage::~DashboardPage() {}
void DashboardPage::updateBalance(double balance, const QString& fullName)
{
	currentBalance = balance;
	ui.greeting->setText("Hi, " + fullName);
	if (balanceVisible) {
		ui.balanceAmount->setText(QString("₹ %1").arg(balance, 0, 'f', 2));
	}
	else {
		ui.balanceAmount->setText("₹ ••••");
	}
}
void DashboardPage::onShowBalanceClicked()
{
	bool ok;
	QString pin = QInputDialog::getText(this, "Transaction PIN",
		"Enter 4-digit PIN:", QLineEdit::Password, "", &ok);
	if (!ok || pin.length() != 4) {
		showErrorMessage("Invalid Input", "Please enter a valid 4-digit PIN.");
		return;
	}
	if (pin != "1234") {
		showErrorMessage("Access Denied", "Incorrect Transaction PIN.");
		return;
	}
	balanceVisible = true;
	updateBalance(currentBalance, "Nasir");
	QTimer::singleShot(15000, this, [this] {
		balanceVisible = false;
		updateBalance(currentBalance, "Nasir");
	});
}
void DashboardPage::onAddFundsConfirmClicked()
{
	double amount = ui.amountInput->value();
	if (amount <= 0) {
		showErrorMessage("Invalid Amount", "Please enter amount greater than zero.");
		return;
	}
	currentBalance += amount;
	updateBalance(currentBalance, "Nasir");
	showSuccessMessage("Success", QString("₹%1 has been added to your wallet.").arg(amount, 0, 'f', 2));
}
void DashboardPage::onSendMoneyClicked()
{
	ui.stackedWidget->setCurrentWidget(ui.P2PTransfer);
}
void DashboardPage::onPayNowClicked()
{
	QString mobile = ui.mobileNumber->text().trimmed();
	double amount = ui.p2pAmount->value();
	if (mobile.isEmpty()) {
		showErrorMessage("Missing Field", "Please enter mobile number.");
		return;
	}
	if (amount <= 0) {
		showErrorMessage("Invalid Amount", "Please enter a valid amount.");
		return;
	}
	if (amount > currentBalance) {
		showErrorMessage("Insufficient Balance", "You do not have enough balance for this transaction.");
		return;
	}
	currentBalance -= amount;
	updateBalance(currentBalance, "Nasir");
	showSuccessMessage("Transaction Completed",
		QString("₹%1 sent successfully to %2").arg(amount, 0, 'f', 2).arg(mobile));
	loadDummyData();
}
void DashboardPage::onRechargeClicked()
{
	QString number = ui.rechargeNumber->text().trimmed();
	double amount = ui.rechargeAmount->value();
	if (number.isEmpty()) {
		showErrorMessage("Missing Field", "Please enter mobile number.");
		return;
	}
	if (amount <= 0) {
		showErrorMessage("Invalid Amount", "Please enter a valid recharge amount.");
		return;
	}
	showSuccessMessage("Recharge Successful",
		QString("Recharge of ₹%1 for %2 completed.").arg(amount, 0, 'f', 2).arg(number));
}
void DashboardPage::setupSidebarNavigation()
{
	QList<QPushButton*> buttons = {
		ui.Home_pushButton, ui.Dashboard_pushButton, ui.Transfer_pushButton,
		ui.P2PTransfer_pushButton, ui.Bills_pushButton,
		ui.Recharge_pushButton, ui.Rewards_pushButton
	};
	for (int i = 0; i < buttons.size(); ++i) {
		connect(buttons[i], &QPushButton::clicked, this, [this, i, buttons] {
			ui.stackedWidget->setCurrentIndex(i);
			for (auto* btn : buttons) btn->setChecked(false);
			buttons[i]->setChecked(true);
			});
	}
}
void DashboardPage::refreshBalance()
{
	double change = (QRandomGenerator::global()->bounded(60) - 20) / 10.0;
	currentBalance += change;
	if (balanceVisible) {
		updateBalance(currentBalance, "Nasir");
	}
}
void DashboardPage::loadDummyData()
{
	ui.transactionsList->clear();
	ui.billsList->clear();
	ui.rewardsList->clear();
	ui.transactionsList->addItems({
		"09/11/2025 11:40 pm Sent to Vikas -₹10.00",
		"09/11/2025 10:48 pm Sent to Vikas -₹12.00",
		"09/11/2025 09:43 pm Received from Vikas +₹12.00",
		"09/11/2025 09:39 pm Sent to Vikas -₹5.00"
		});
	ui.billsList->addItems({
		"Electricity Bill - ₹1,245 Due on 05 Apr 2026",
		"Broadband Bill - ₹899 Due on 12 Apr 2026",
		"Mobile Postpaid - ₹599 Due on 28 Mar 2026"
		});
	ui.rewardsList->addItems({
		"₹200 Cashback on UPI Transactions",
		"Free Recharge worth ₹100 on ₹500+ spend",
		"Referral Bonus - ₹150 pending"
		});
}
void DashboardPage::showSuccessMessage(const QString& title, const QString& message)
{
	QMessageBox::information(this, title, message);
}
void DashboardPage::showErrorMessage(const QString& title, const QString& message)
{
	QMessageBox::warning(this, title, message);
}
void DashboardPage::onLogoutClicked()
{
	if (QMessageBox::question(this, "Logout", "Are you sure you want to logout?") == QMessageBox::Yes) {
		emit logoutRequested();
	}
}