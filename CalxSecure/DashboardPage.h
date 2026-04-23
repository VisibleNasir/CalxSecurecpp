#pragma once

#include <QWidget>
#include "ui_DashboardPage.h"

class DashboardPage : public QWidget
{
	Q_OBJECT

public:
	DashboardPage(QWidget *parent = nullptr);
	~DashboardPage();
public:
	void updateBalance(double balance, const QString& fullName);
private:
	Ui::DashboardPageClass ui;
};

