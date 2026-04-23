#pragma once

#include <QWidget>
#include "ui_Home.h"

class Home : public QWidget
{
	Q_OBJECT

public:
	explicit Home(QWidget *parent = nullptr);
	~Home();
	void updateBalanceDisplay();

signals:
	void viewDashboardRequested(); 

private:
	Ui::HomeClass ui;
};

