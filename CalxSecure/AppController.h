#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include "AuthManager.h"
#include "Home.h"
#include "components/NavBar.h"
#include "components/DockWidget.h"
#include "DashboardPage.h"

class AppController : public QMainWindow
{
	Q_OBJECT

public:
    explicit AppController(QWidget* parent = nullptr);
    void closeEvent(QCloseEvent* event) override;
    void switchTheme(bool dark);
    void showLoginDialog();

protected:
    void setupUI();
    void setupNavigation();

private:
    Home* m_homePage;
    NavBar* m_navBar;
    DockWidget* m_dock;
    QStackedWidget* stackedWidget;
    AuthManager* authManager;
    DashboardPage* m_dashboardPage = nullptr;
};

