// navigation/NavBar.h
#pragma once
#include <QWidget>
#include <QHBoxLayout>
#include <QStackedWidget>
#include "AnimatedButton.h"

class NavBar : public QWidget {
    Q_OBJECT
public:
    explicit NavBar(QStackedWidget* stackedWidget, QWidget* parent = nullptr);

signals:
    void homeRequested();

private:
    QHBoxLayout* m_layout;
    QStackedWidget* m_stackedWidget;
    AnimatedButton* m_btnHome;
    AnimatedButton* m_btnTransfer;
    AnimatedButton* m_btnBills;
    AnimatedButton* m_btnRecharge;
    AnimatedButton* m_btnRewards;
};