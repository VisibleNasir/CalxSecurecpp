#include "GlobalStyle.h"
#include <QApplication>
#include <QPalette>
#include <QStyleFactory>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "AppController.h"
#include "Home.h"          // ← Your new Home page
#include "components/NavBar.h"
#include "components/DockWidget.h"
#include <QTimer>
#include "LoginDialog.h"

AppController::AppController(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("CalxSecure");
    resize(1440, 920);

    authManager = new AuthManager(this);

    setupUI();
    setupNavigation();

    // Database Connection
    if (!DatabaseManager::instance().connect()) {
        QMessageBox::critical(this, "Fatal Error",
            "Cannot connect to database.\nPlease check PostgreSQL is running and schema is applied.");
        qApp->quit();
        return;
    }

    // Apply dark theme by default
    switchTheme(true);

    // Optional: Auto login for testing (remove in production)
    QTimer::singleShot(500, this, [this]() {
        authManager->login("nasir@example.com", "password123", "user");
        });
}
void AppController::showLoginDialog()
{
    LoginDialog* dialog = new LoginDialog(this);

    connect(dialog, &LoginDialog::loginRequested, this,
        [this](const QString& email, const QString& pass, const QString& role) {
            if (authManager->login(email, pass, role)) {
                // Success - WebSocket will connect automatically via signal
            }
            else {
                QMessageBox::warning(this, "Login Failed", "Invalid credentials");
            }
        });

    //dialog->exec();
}

void AppController::switchTheme(bool dark)
{
    QString style = dark ? GlobalStyle::getDarkTheme() : GlobalStyle::getLightTheme();
    qApp->setStyleSheet(style);

    QPalette palette = dark
        ? QPalette(QColor("#0f0f12"), QColor("#1a1a1f"))
        : QPalette(QColor("#f8f9fa"));
    qApp->setPalette(palette);
}

void AppController::setupUI()
{
    // Create stacked widget for multiple pages
    stackedWidget = new QStackedWidget(this);

    // Add your new Home page (designed with .ui file)
    m_homePage = new Home(this);           // Home class from ui_Home.h
    stackedWidget->addWidget(m_homePage);

    // TODO: Add more pages later (P2PPage, BillsPage, etc.)
    // stackedWidget->addWidget(m_p2pPage);

    stackedWidget->setCurrentIndex(0);
}

void AppController::setupNavigation()
{
    // Left Dock (macOS style)
    m_dock = new DockWidget(stackedWidget, this);

    // Top Navigation Bar (Floating style)
    m_navBar = new NavBar(stackedWidget, this);

    // Main container with Left Dock + Content Area
    QWidget* mainContainer = new QWidget(this);
    QHBoxLayout* hLayout = new QHBoxLayout(mainContainer);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);

    hLayout->addWidget(m_dock);
    hLayout->addWidget(stackedWidget, 1);   // Content takes remaining space

    // Optional: Add Top NavBar above everything
    QVBoxLayout* mainVertical = new QVBoxLayout();
    mainVertical->setContentsMargins(0, 0, 0, 0);
    mainVertical->setSpacing(0);
    mainVertical->addWidget(m_navBar);
    mainVertical->addWidget(mainContainer);

    // Set as central widget
    QWidget* rootWidget = new QWidget(this);
    rootWidget->setLayout(mainVertical);
    setCentralWidget(rootWidget);

    // Connect signals
    connect(m_dock, &DockWidget::pageRequested,
        stackedWidget, &QStackedWidget::setCurrentIndex);

    connect(m_navBar, &NavBar::homeRequested, this, [this]() {
        stackedWidget->setCurrentIndex(0);
        });

    // Connect buttons from Home.ui Top Dock to navigation
    //connect(m_homePage->ui.btnTransfer, &QPushButton::clicked, this, [this]() {
    //    // You can change index when you add P2P page (e.g. index 1)
    //    stackedWidget->setCurrentIndex(1);   // Change later when more pages added
    //    });

    //connect(m_homePage->ui.btnBills, &QPushButton::clicked, this, [this]() {
    //    stackedWidget->setCurrentIndex(2);   // Future Bills page
    //    });

    //connect(m_homePage->ui.btnRecharge, &QPushButton::clicked, this, [this]() {
    //    stackedWidget->setCurrentIndex(3);
    //    });

    //connect(m_homePage->ui.btnRewards, &QPushButton::clicked, this, [this]() {
    //    stackedWidget->setCurrentIndex(4);
    //    });
}