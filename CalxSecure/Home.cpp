#include "Home.h"
#include "core/AppState.h"

Home::Home(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    // Clicking the main gradient button routes you to the dashboard
    connect(ui.btnGetStarted, &QPushButton::clicked, this, &Home::viewDashboardRequested);
}

Home::~Home()
{
}