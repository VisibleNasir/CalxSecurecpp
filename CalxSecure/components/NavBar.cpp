// navigation/NavBar.cpp
#include "NavBar.h"
#include <QSpacerItem>

NavBar::NavBar(QStackedWidget* stackedWidget, QWidget* parent)
    : QWidget(parent), m_stackedWidget(stackedWidget)
{
    setFixedHeight(70);
    setStyleSheet(R"(
        QWidget {
            background-color: rgba(26, 26, 31, 0.95);
            border-bottom: 1px solid #2a2a32;
        }
    )");

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(40, 0, 40, 0);
    m_layout->setSpacing(12);

    m_btnHome = new AnimatedButton("Home", this);
    m_btnTransfer = new AnimatedButton("Transfer", this);
    m_btnBills = new AnimatedButton("Bills", this);
    m_btnRecharge = new AnimatedButton("Recharge", this);
    m_btnRewards = new AnimatedButton("Rewards", this);

    // Style secondary buttons
    QString secondaryStyle = "QPushButton { background-color: #1f1f26; border: 1px solid #3a3a44; }";
    m_btnTransfer->setStyleSheet(secondaryStyle);
    m_btnBills->setStyleSheet(secondaryStyle);
    m_btnRecharge->setStyleSheet(secondaryStyle);
    m_btnRewards->setStyleSheet(secondaryStyle);

    m_layout->addWidget(m_btnHome);
    m_layout->addWidget(m_btnTransfer);
    m_layout->addWidget(m_btnBills);
    m_layout->addWidget(m_btnRecharge);
    m_layout->addWidget(m_btnRewards);
    m_layout->addStretch();

    // Connections (we will connect pages in AppController)
    connect(m_btnHome, &QPushButton::clicked, this, &NavBar::homeRequested);
}