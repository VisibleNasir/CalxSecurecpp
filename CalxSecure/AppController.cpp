#include "AppController.h"
#include "Home.h"
#include "DashboardPage.h"
#include "LoginDialog.h"
#include "core/AppState.h"
#include <QApplication>
#include <QPalette>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QTimer>
#include <QSettings>
#include <QCloseEvent>

AppController::AppController(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("CalxSecure");
    resize(1440, 920);

    authManager = new AuthManager(this);

    setupUI();
    setupNavigation();

    if (!DatabaseManager::instance().connect()) {
        QMessageBox::critical(this, "Fatal Error",
            "Cannot connect to database.\nCheck PostgreSQL is running and tables exist.");
        qApp->quit();
        return;
    }

    switchTheme(true);
    QTimer::singleShot(200, this, &AppController::showLoginDialog);
}

void AppController::setupUI()
{
    stackedWidget = new QStackedWidget(this);
    m_homePage = new Home(this);
    m_dashboardPage = new DashboardPage(this);

    stackedWidget->addWidget(m_homePage);     // 0
    stackedWidget->addWidget(m_dashboardPage); // 1
    connect(m_dashboardPage, &DashboardPage::logoutRequested, this, [this]() {
        authManager->logout();
        this->hide();
        showLoginDialog();
        });
    // Restore window geometry
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

    connect(m_homePage, &Home::loginRequested, this, &AppController::showLoginDialog);
}

void AppController::showLoginDialog()
{
    this->hide();

    LoginDialog* dialog = new LoginDialog(nullptr);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    // Login
    connect(dialog, &LoginDialog::loginRequested, this,
        [this, dialog](const QString& email, const QString& pass, const QString& role) {
            if (authManager->login(email, pass, role)) {
                const auto& s = authManager->currentSession();

                SessionData sd;
                sd.userId = QString::number(s.userId);
                sd.fullName = s.fullName;
                sd.email = s.email;
                sd.balance = s.balance;

                AppState::instance().setSession(sd);
                m_dashboardPage->updateBalance(s.balance, s.fullName);

                this->show();
                stackedWidget->setCurrentIndex(1); // Dashboard

                // Use hide and deleteLater instead of close() since closeEvent is ignored
                dialog->hide();
                dialog->deleteLater();
            }
            else {
                QMessageBox::warning(dialog, "Login Failed", "Invalid credentials");
            }
        });

    // Signup
    connect(dialog, &LoginDialog::signupRequested, this,
        [this, dialog](const QString& username, const QString& email, const QString& pass,
            const QString& fullName, const QString& phone) {
                if (authManager->signup(username, email, pass, fullName, phone, "user")) {
                    QMessageBox::information(dialog, "Success",
                        "Account created successfully!\nPlease sign in.");
                    dialog->switchToLoginMode();
                }
        });

    connect(authManager, &AuthManager::signupFailed, dialog,
        [dialog](const QString& msg) {
            QMessageBox::warning(dialog, "Signup Failed", msg);
        });

    dialog->show();
}

void AppController::switchTheme(bool /*dark*/)
{
    QString globalStyle = R"(
        QWidget { background-color: #050505; color: #ffffff; font-family: 'Segoe UI', Inter, sans-serif; }
        QLabel { background-color: transparent; color: #ffffff; }
        QPushButton {
            background-color: #1a1a1f; color: #ffffff;
            border: 1px solid #333340; border-radius: 8px; padding: 8px 16px;
        }
        QPushButton:hover {
            background-color: #2a2a35; border: 1px solid #5c5cff;
        }
        QLineEdit, QDoubleSpinBox, QComboBox {
            background-color: #0f0f12; color: #ffffff;
            border: 1px solid #2a2a35; border-radius: 8px; padding: 8px;
        }
        QScrollArea { border: none; background-color: transparent; }
    )";
    qApp->setStyleSheet(globalStyle);

    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#050505"));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor("#0f0f12"));
    palette.setColor(QPalette::Text, Qt::white);
    qApp->setPalette(palette);
}

void AppController::closeEvent(QCloseEvent* event)
{
    QSettings settings("CalxSecure", "CalxSecureApp");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    QMainWindow::closeEvent(event);
}