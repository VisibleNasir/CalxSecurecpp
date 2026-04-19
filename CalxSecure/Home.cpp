#include "Home.h"

Home::Home(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    // Connect top dock buttons
    connect(ui.btnTransfer, &QPushButton::clicked, this, &Home::transferRequested);
    connect(ui.btnBills, &QPushButton::clicked, this, &Home::billsRequested);
    connect(ui.btnRecharge, &QPushButton::clicked, this, &Home::rechargeRequested);
    connect(ui.btnRewards, &QPushButton::clicked, this, &Home::rewardsRequested);
}

Home::~Home()
{}

