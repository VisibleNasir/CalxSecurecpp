#include "AppController.h"
#include "Home.h"
#include "DashboardPage.h"
#include "LoginDialog.h"
#include "core/AppState.h"
#include <QApplication>
#include <QPalette>
#include <QVBoxLayout>
#include <QTimer>
#include <QSettings>
#include <QCloseEvent>
#include "DBSchema.h"
#include <QInputDialog>
#include <QMessagebox>
#include "components/PinDialog.h"

AppController::AppController(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("CalxSecure");
    resize(1440, 920);

    authManager = new AuthManager(this);

    setupUI();
    setupNavigation();

    // DB check (no popup → exit silently/log)
    if (!DatabaseManager::instance().connect()) {
        qCritical() << "[FATAL] Database connection failed";
        qApp->quit();
        return;
    }
    if (DatabaseManager::instance().connect()) {
        QSqlDatabase db = QSqlDatabase::database("calxsecure_main");
        DBSchema::initialize(db);
    }
    switchTheme(true);

    // Try to auto-login, otherwise show login dialog
    QTimer::singleShot(200, this, [this]() {
        QSettings sessionSettings("CalxSecure", "Session");
        QString savedEmail = sessionSettings.value("email").toString();
        QString savedPassword = sessionSettings.value("password").toString(); // Warning: Storing plain text pass in QSettings is insecure in production

        if (!savedEmail.isEmpty() && !savedPassword.isEmpty()) {
            if (authManager->login(savedEmail, savedPassword, "user")) {
                const auto& s = authManager->currentSession();

                SessionData sd;
                sd.userId = QString::number(s.userId);
                sd.fullName = s.fullName;
                sd.email = s.email;
                sd.balance = s.balance;

                AppState::instance().setSession(sd);
                m_dashboardPage->updateBalance(s.balance, s.fullName);
                this->show();
                stackedWidget->setCurrentIndex(1); // Take directly to dashboard

                // First time PIN check will be executed inside DashboardPage when needed
                return;
            }
        }

        // If auto-login fails or credentials don't exist, show login screen
        showLoginDialog();
    });
}

void AppController::setupUI()
{
    stackedWidget = new QStackedWidget(this);
    m_homePage = new Home(this);
    m_dashboardPage = new DashboardPage(this);
    m_adminPage = new AdminPage(this);
    stackedWidget->addWidget(m_homePage);
    stackedWidget->addWidget(m_dashboardPage);
    stackedWidget->addWidget(m_adminPage);

    connect(m_dashboardPage, &DashboardPage::homeRequested, this, [this]() {
        stackedWidget->setCurrentIndex(0); // 0 = Home page
        });
    // Logout handling
    connect(m_dashboardPage, &DashboardPage::logoutRequested, this, [this]() {
        // Clear saved credentials on logout
        QSettings sessionSettings("CalxSecure", "Session");
        sessionSettings.remove("email");
        sessionSettings.remove("password");

        authManager->logout();
        this->hide();
        showLoginDialog();
    });

    // Restore window
    QSettings settings("CalxSecure", "CalxSecureApp");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

void AppController::setupNavigation()
{
    QWidget* root = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(root);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    mainLayout->addWidget(stackedWidget);
    setCentralWidget(root);
    connect(m_homePage, &Home::dashboardRequested, this, [this]() {
        stackedWidget->setCurrentIndex(1);
        m_dashboardPage->setCurrentTab(0);
        });
    connect(m_homePage, &Home::transferRequested, this, [this]() {
        stackedWidget->setCurrentIndex(1);
        m_dashboardPage->setCurrentTab(1);
        });
    connect(m_homePage, &Home::p2pRequested, this, [this]() {
        stackedWidget->setCurrentIndex(1);
        m_dashboardPage->setCurrentTab(2);
        });
    connect(m_homePage, &Home::analyticsRequested, this, [this]() {
        // Just jumping to dashboard bills or other relevant view, 
        // fallback to dashboard if analytics tab doesn't exist explicitly 
        stackedWidget->setCurrentIndex(1);
        m_dashboardPage->setCurrentTab(0); 
        });

    connect(m_homePage, &Home::loginRequested, this, &AppController::showLoginDialog);
}

void AppController::showLoginDialog()
{
    this->hide();

    LoginDialog* dialog = new LoginDialog(nullptr);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    // LOGIN
    connect(dialog, &LoginDialog::loginRequested, this,
        [this, dialog](const QString& email, const QString& pass, const QString& role) {

            if (authManager->login(email, pass, role)) {
                // Save credentials
                QSettings sessionSettings("CalxSecure", "Session");
                sessionSettings.setValue("email", email);
                sessionSettings.setValue("password", pass);

                const auto& s = authManager->currentSession();
                SessionData sd;
                sd.userId = QString::number(s.userId);
                sd.fullName = s.fullName;
                sd.email = s.email;
                sd.balance = s.balance;
                AppState::instance().setSession(sd);

                this->show();

                // ROLE-BASED REDIRECTION
                if (s.role == "Admin") { 
                    stackedWidget->setCurrentWidget(m_adminPage);
                    qInfo() << "✅ Admin logged in - showing Admin Panel";
                }
                else {
                    stackedWidget->setCurrentWidget(m_dashboardPage);
                    m_dashboardPage->updateBalance(s.balance, s.fullName);
                    qInfo() << "✅ User logged in - showing Dashboard";
                }

                dialog->deleteLater();
                return;
            }
        });

    // SIGNUP
    connect(dialog, &LoginDialog::signupRequested, this,
        [this, dialog](const QString& username, const QString& email,
            const QString& pass, const QString& fullName,
            const QString& phone) {

                if (authManager->signup(username, email, pass, fullName, phone, "user")) {
                    
                    // Immediately try logging them in with the new credentials
                    if (authManager->login(email, pass, "user")) {
                        // Save newly created credentials for auto-login later
                        QSettings sessionSettings("CalxSecure", "Session");
                        sessionSettings.setValue("email", email);
                        sessionSettings.setValue("password", pass);

                        const auto& s = authManager->currentSession();

                        SessionData sd;
                        sd.userId = QString::number(s.userId);
                        sd.fullName = s.fullName;
                        sd.email = s.email;
                        sd.balance = s.balance;

                        AppState::instance().setSession(sd);
                        m_dashboardPage->updateBalance(s.balance, s.fullName);
                        this->show(); // Show window
                        stackedWidget->setCurrentIndex(1); // Jump straight to Dashboard
                        dialog->deleteLater(); // Get rid of login popup
                    } else {
                        // Fallback if somehow login directly fails 
                        dialog->resetForm();
                        dialog->showError("Signup successful, but login failed.");
                        dialog->switchToLoginMode();
                    }
                } else {
                    dialog->resetForm();
                    dialog->showError("Signup failed! Account may already exist.");
                }
        });

    connect(authManager, &AuthManager::signupFailed, dialog,
        [dialog](const QString& msg) {
            dialog->resetForm();
            dialog->showError(msg);
        });

    dialog->show();
}

void AppController::switchTheme(bool)
{
    QString globalStyle = R"(
        QWidget { background-color: #050505; color: #ffffff; font-family: 'Segoe UI'; }
        QPushButton {
            background-color: #1a1a1f;
            border: 1px solid #333;
            border-radius: 8px;
            padding: 6px 12px;
        }
        QPushButton:hover {
            border: 1px solid #5c5cff;
        }
        QLineEdit, QComboBox {
            background-color: #0f0f12;
            border: 1px solid #2a2a35;
            border-radius: 8px;
            padding: 6px;
        }
    )";

    qApp->setStyleSheet(globalStyle);
}

void AppController::closeEvent(QCloseEvent* event)
{
    QSettings settings("CalxSecure", "CalxSecureApp");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    QMainWindow::closeEvent(event);
}