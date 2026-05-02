#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QMainWindow>
#include <QStackedWidget>
#include "AdminPage.h"
#include "AuthManager.h"
class Home;
class DashboardPage;

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
    AdminPage* m_adminPage = nullptr;
    DashboardPage* m_dashboardPage = nullptr;
    AuthManager* authManager = nullptr;

};

#endif // APPCONTROLLER_H