// navigation/DockWidget.cpp
#include "DockWidget.h"
#include <QToolButton>
#include <QGraphicsDropShadowEffect>
#include <QStackedWidget>

DockWidget::DockWidget(QStackedWidget* stacked, QWidget* parent)
    : QWidget(parent), m_stackedWidget(stacked)
{
    setFixedWidth(80);
    setStyleSheet("background-color: #1a1a1f; border-right: 1px solid #2a2a32;");

    m_layout = new QVBoxLayout(this);
    m_layout->setSpacing(20);
    m_layout->setContentsMargins(10, 40, 10, 40);
    m_layout->addStretch();

    // Dock buttons
    createDockButton("🏠", "Home", 0);
    createDockButton("↔️", "P2P Transfer", 1);
    createDockButton("📄", "Bills", 2);
    createDockButton("🔋", "Recharge", 3);
    createDockButton("🏆", "Rewards", 4);

    m_layout->addStretch();
}

void DockWidget::createDockButton(const QString& iconText, const QString& tooltip, int pageIndex)
{
    auto* btn = new QToolButton(this);
    btn->setText(iconText);
    btn->setToolTip(tooltip);
    btn->setFixedSize(56, 56);
    btn->setStyleSheet(R"(
        QToolButton {
            font-size: 28px;
            background-color: #25252b;
            border: none;
            border-radius: 16px;
        }
        QToolButton:hover {
            background-color: #00b4d8;
            color: black;
        }
    )");

    connect(btn, &QToolButton::clicked, this, [this, pageIndex]() {
        emit pageRequested(pageIndex);
        });

    m_layout->addWidget(btn);
}