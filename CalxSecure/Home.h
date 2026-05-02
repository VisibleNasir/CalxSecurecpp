#pragma once

#include <QWidget>
#include "ui_Home.h"

class Home : public QWidget
{
	Q_OBJECT

public:
    Home(QWidget* parent = nullptr);
    ~Home();
signals:
    void dashboardRequested();

private slots:
    void onPrimaryBtnClicked();
signals:
void transferRequested();
void p2pRequested();
void analyticsRequested();
void billsRequested();
	void rechargeRequested();
	void rewardsRequested();
    void loginRequested();
private:
    Ui::HomeClass ui;
    void setupAnimations();
};