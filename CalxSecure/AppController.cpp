#include "GlobalStyle.h"
#include <QApplication>
#include <QPalette>
#include <QStyleFactory>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "AppController.h"
#include "Home.h"
#include "DashboardPage.h"
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
    QSettings settings("CalxSecure", "CalxSecureApp");

    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    stackedWidget = new QStackedWidget(this);

    m_homePage = new Home(this);
    m_dashboardPage = new DashboardPage(this);

    stackedWidget->addWidget(m_homePage);      // 0
    stackedWidget->addWidget(m_dashboardPage); // 1
}
void AppController::closeEvent(QCloseEvent* event)
{
    QSettings settings("CalxSecure", "CalxSecureApp");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    QMainWindow::closeEvent(event);
}
void AppController::setupNavigation()
{
    QWidget* root = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(root);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ===== FULL SCREEN CONTENT ONLY =====
    mainLayout->addWidget(stackedWidget);

    setCentralWidget(root);

    // ===== HOME CTA → LOGIN =====
    connect(m_homePage, &Home::loginRequested,
        this, &AppController::showLoginDialog);
}
void AppController::showLoginDialog()
{
    this->hide();

    LoginDialog* dialog = new LoginDialog(nullptr);  // no parent = independent window
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    // === SAFE CONNECTIONS ===
    connect(dialog, &LoginDialog::loginRequested, this,
        [this, dialog](const QString& email, const QString& pass, const QString& role) {
            if (authManager->login(email, pass, role)) {
                const auto& s = authManager->currentSession();

                SessionData sd;
                sd.userId = QString::number(s.userId);
                sd.fullName = s.fullName;
                sd.email = s.email;
                sd.balance = s.balance;   // you'll add balance later

                AppState::instance().setSession(sd);
                m_dashboardPage->updateBalance(s.balance, s.fullName);

                this->show();
                stackedWidget->setCurrentIndex(1); // Dashboard

                dialog->close();   // safer than deleteLater() here
            }
            else {
                QMessageBox::warning(dialog, "Login Failed", "Invalid credentials or database error");
            }
        });

    connect(dialog, &LoginDialog::signupRequested, this,
        [this, dialog](const QString& username, const QString& email,
            const QString& pass, const QString& fullName, const QString& phone) {

                if (authManager->signup(username, email, pass, fullName, phone, "user")) {
                    QMessageBox::information(dialog, "Success",
                        "Account created! Please sign in with your new credentials.");
                    dialog->switchToLoginMode();
                }
                // signupFailed signal is already connected in your code
        });

    // Forward AuthManager errors to dialog
    connect(authManager, &AuthManager::signupFailed, dialog,
        [dialog](const QString& msg) {
            QMessageBox::warning(dialog, "Signup Failed", msg);
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