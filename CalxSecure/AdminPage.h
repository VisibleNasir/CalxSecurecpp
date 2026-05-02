#pragma once
#include <QWidget>
#include "ui_AdminPage.h"

class AdminPage : public QWidget
{
    Q_OBJECT

public:
    explicit AdminPage(QWidget* parent = nullptr);
    ~AdminPage();

private slots:
    void loadPendingOnramp();
    void onApproveSelected();
    void onRejectSelected();
    void onRefreshClicked();

private:
    Ui::AdminPageClass ui;
    void setupUI();
    void setupConnections();
};