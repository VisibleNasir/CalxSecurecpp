#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QMainWindow>
#include <QStackedWidget>
#include "AuthManager.h"
#include "Home.h"
#include "DashboardPage.h"
#include "P2PPage.h"

class AppController : public QMainWindow
{
    Q_OBJECT

public:
    explicit AppController(QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupUI();
    void setupNavigation();
    void showLoginDialog();
    void switchTheme(bool dark);

    QStackedWidget* stackedWidget = nullptr;
    Home* m_homePage = nullptr;
    DashboardPage* m_dashboardPage = nullptr;
    P2PPage* m_p2pPage = nullptr;
    AuthManager* authManager = nullptr;
};

#endif // APPCONTROLLER_H
