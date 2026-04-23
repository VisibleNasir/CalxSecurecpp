#include "DashboardPage.h"

DashboardPage::DashboardPage(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

DashboardPage::~DashboardPage()
{}

void DashboardPage::updateBalance(double balance, const QString& fullName)
{
	ui.lblBalance->setText(QString("₹ %1").arg(balance, 0, 'f', 2));
	ui.lblUserName->setText(fullName);
}