#include "Home.h"
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QPushButton>
#include "core/AppState.h"

Home::Home(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    connect(ui.primaryBtn, &QPushButton::clicked,
        this, &Home::viewDashboardRequested);
}

Home::~Home() {}

void Home::onGetStartedClicked()
{
    emit loginRequested();   // 🔥 trigger login flow
}