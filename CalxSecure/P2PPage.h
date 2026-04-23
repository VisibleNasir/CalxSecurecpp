#pragma once

#include <QWidget>
#include "ui_P2PPage.h"

class P2PPage : public QWidget
{
	Q_OBJECT

public:
	explicit P2PPage(QWidget *parent = nullptr);
	~P2PPage();

private slots:
    void onSendClicked();

private:
	Ui::P2PPageClass ui;
};

