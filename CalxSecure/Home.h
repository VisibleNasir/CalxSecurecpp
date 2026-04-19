#pragma once

#include <QWidget>
#include "ui_Home.h"

class Home : public QWidget
{
	Q_OBJECT

public:
	Home(QWidget *parent = nullptr);
	~Home();

signals:
	void transferRequested();
	void billsRequested();
	void rechargeRequested();
	void rewardsRequested();
private:
	Ui::HomeClass ui;
};

