#pragma once

#include <QWidget>

#include "ui_Home.h"

class Home : public QWidget
{
    Q_OBJECT

public:
    Home(QWidget* parent = nullptr);
    ~Home();
    void onGetStartedClicked();
signals:
    void viewDashboardRequested();
    void loginRequested();



private:
    Ui::HomeClass ui;

};