#include "GlobalStyle.h"
#include <QApplication>
#include <QPalette>
#include <QStyleFactory>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include "AppController.h"
#include "Home.h"
#include "DashboardPage.h"
#include "P2PPage.h"
// Add more pages as you create them
// #include "BillsPage.h"
// #include "RechargePage.h"
// #include "RewardsPage.h"

#include "components/NavBar.h"
#include "components/DockWidget.h"
#include "LoginDialog.h"

AppController::AppController(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("CalxSecure");
    resize(1440, 920);

    authManager = new AuthManager(this);

    setupUI();
    setupNavigation();

    if (!DatabaseManager::instance().connect()) {
        QMessageBox::critical(this, "Fatal Error",
            "Cannot connect to database.\nPlease check PostgreSQL is running and schema is applied.");
        qApp->quit();
        return;
    }

    switchTheme(true);

    // Show login first
    QTimer::singleShot(300, this, &AppController::showLoginDialog);
}

void AppController::setupUI()
{
    stackedWidget = new QStackedWidget(this);

    m_homePage = new Home(this);
    m_dashboardPage = new DashboardPage(this);
    m_p2pPage = new P2PPage(this);

    stackedWidget->addWidget(m_homePage);      // Index 0
    stackedWidget->addWidget(m_dashboardPage); // Index 1
    stackedWidget->addWidget(m_p2pPage);       // Index 2

    // Add more pages here later
    // stackedWidget->addWidget(m_billsPage);     // Index 3
    // stackedWidget->addWidget(m_rechargePage);  // Index 4
    // stackedWidget->addWidget(m_rewardsPage);   // Index 5
}

void AppController::setupNavigation()
{
    m_dock = new DockWidget(stackedWidget, this);
    m_navBar = new NavBar(stackedWidget, this);

    QWidget* mainContainer = new QWidget();
    QHBoxLayout* hLayout = new QHBoxLayout(mainContainer);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(m_dock);
    hLayout->addWidget(stackedWidget, 1);

    QVBoxLayout* mainVertical = new QVBoxLayout();
    mainVertical->addWidget(m_navBar);
    mainVertical->addWidget(mainContainer);

    QWidget* root = new QWidget();
    root->setLayout(mainVertical);
    setCentralWidget(root);

    // Connections
    connect(m_dock, &DockWidget::pageRequested, stackedWidget, &QStackedWidget::setCurrentIndex);
    connect(m_navBar, &NavBar::homeRequested, this, [this]() { stackedWidget->setCurrentIndex(0); });

    // Connect from Home page buttons
    connect(m_homePage, &Home::viewDashboardRequested, this, [this]() { stackedWidget->setCurrentIndex(1); }); // Dashboard
    // Add more connections as needed
}

void AppController::showLoginDialog()
{
    LoginDialog* dialog = new LoginDialog(this);
    connect(dialog, &LoginDialog::loginRequested, this,
        [this](const QString& email, const QString& pass, const QString& role) {
            if (authManager->login(email, pass, role)) {
                // Update dashboard with user info
                auto& session = authManager->currentSession();
                m_dashboardPage->updateBalance(session.balance, session.fullName);
                stackedWidget->setCurrentIndex(1); // Go to Dashboard after login
            }
            else {
                QMessageBox::warning(this, "Login Failed", "Invalid credentials");
            }
        });
    //dialog->exec();
}

void AppController::switchTheme(bool dark)
{
    // Enforce strict global dark theme for the entire application
    QString globalStyle = R"(
        QWidget {
            background-color: #050505;
            color: #ffffff;
            font-family: 'Segoe UI', Inter, sans-serif;
        }
        QLabel {
            background-color: transparent;
            color: #ffffff;
        }
        QPushButton {
            background-color: #1a1a1f;
            color: #ffffff;
            border: 1px solid #333340;
            border-radius: 8px;
            padding: 8px 16px;
        }
        QPushButton:hover {
            background-color: #2a2a35;
            border: 1px solid #5c5cff;
        }
        QLineEdit, QDoubleSpinBox {
            background-color: #0f0f12;
            color: #ffffff;
            border: 1px solid #2a2a35;
            border-radius: 8px;
            padding: 8px;
        }
        QLineEdit:focus, QDoubleSpinBox:focus {
            border: 1px solid #5c5cff;
        }
        QScrollArea {
            border: none;
            background-color: transparent;
        }
    )";
    
    qApp->setStyleSheet(globalStyle);

    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#050505"));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor("#0f0f12"));
    palette.setColor(QPalette::Text, Qt::white);
    qApp->setPalette(palette);
}