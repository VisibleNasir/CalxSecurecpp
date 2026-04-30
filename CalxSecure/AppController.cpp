#include "AppController.h"
#include "Home.h"
#include "DashboardPage.h"
#include "LoginDialog.h"
#include "core/AppState.h"
#include <QApplication>
#include <QPalette>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QGraphicsBlurEffect>
#include <QSettings>
#include <QCloseEvent>

#include "AppController.h"
#include "Home.h"
#include "DashboardPage.h"
#include "P2PPage.h"
#include "LoginDialog.h"
#include "core/AppState.h"

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

    // Show login after UI loads
    QTimer::singleShot(200, this, &AppController::showLoginDialog);
}
void AppController::setupUI()
{
    stackedWidget = new QStackedWidget(this);

    m_homePage = new Home(this);
    m_dashboardPage = new DashboardPage(this);
    m_p2pPage = new P2PPage(this);

    stackedWidget->addWidget(m_homePage);      // 0
    stackedWidget->addWidget(m_dashboardPage); // 1
    stackedWidget->addWidget(m_p2pPage);       // 2

    // Restore window geometry
    QSettings settings("CalxSecure", "CalxSecureApp");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}
void AppController::setupNavigation()
{
    QWidget* root = new QWidget(this);
    QHBoxLayout* mainLayout = new QHBoxLayout(root);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ===== SIDEBAR =====
    QFrame* sidebar = new QFrame();
    sidebar->setObjectName("sidebar");
    sidebar->setMinimumWidth(80);
    sidebar->setMaximumWidth(280);

    QVBoxLayout* sideLayout = new QVBoxLayout(sidebar);

    QPushButton* btnToggle = new QPushButton("≡");
    QPushButton* btnHome = new QPushButton("Home");
    QPushButton* btnDashboard = new QPushButton("Dashboard");
    QPushButton* btnP2P = new QPushButton("Payments");

    sideLayout->addWidget(btnToggle);
    sideLayout->addWidget(btnHome);
    sideLayout->addWidget(btnDashboard);
    sideLayout->addWidget(btnP2P);
    sideLayout->addStretch();

    // ===== CONTENT =====
    QWidget* content = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->addWidget(stackedWidget);

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(content, 1);

    setCentralWidget(root);

    // ===== NAVIGATION =====
    connect(btnHome, &QPushButton::clicked, this, [this]() {
        stackedWidget->setCurrentIndex(0);
        });

    connect(btnDashboard, &QPushButton::clicked, this, [this]() {
        if (!AppState::instance().isLoggedIn()) {
            showLoginDialog();
            return;
        }
        stackedWidget->setCurrentIndex(1);
        });

    connect(btnP2P, &QPushButton::clicked, this, [this]() {
        if (!AppState::instance().isLoggedIn()) {
            showLoginDialog();
            return;
        }
        stackedWidget->setCurrentIndex(2);
        });

    // ===== SIDEBAR ANIMATION =====
    bool isCollapsed = false;

    connect(btnToggle, &QPushButton::clicked, this, [=]() mutable {

        QPropertyAnimation* anim = new QPropertyAnimation(sidebar, "maximumWidth");
        anim->setDuration(220);
        anim->setEasingCurve(QEasingCurve::InOutCubic);

        if (isCollapsed) {
            anim->setEndValue(280);
            btnHome->setText("Home");
            btnDashboard->setText("Dashboard");
            btnP2P->setText("Payments");
        }
        else {
            anim->setEndValue(80);
            btnHome->setText("");
            btnDashboard->setText("");
            btnP2P->setText("");
        }

        anim->start(QAbstractAnimation::DeleteWhenStopped);
        isCollapsed = !isCollapsed;
        });

    // ===== HOME CTA → LOGIN =====
    connect(m_homePage, &Home::loginRequested,
        this, &AppController::showLoginDialog);
}
void AppController::showLoginDialog()
{
    // Apply blur effect to background
    QGraphicsBlurEffect* blurEffect = new QGraphicsBlurEffect(this);
    blurEffect->setBlurRadius(15);
    stackedWidget->setGraphicsEffect(blurEffect);

    LoginDialog* dialog = new LoginDialog(this);
    dialog->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    dialog->setWindowModality(Qt::ApplicationModal);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    // Remove blur when dialog is removed
    connect(dialog, &QObject::destroyed, this, [this]() {
        stackedWidget->setGraphicsEffect(nullptr);
        });

    // Handle Login
    connect(dialog, &LoginDialog::loginRequested, this,
        [this, dialog](const QString& email, const QString& pass, const QString& role) {

            if (authManager->login(email, pass, role)) {
                auto& s = authManager->currentSession();
                SessionData sessionData;
                sessionData.userId = QString::number(s.userId);
                sessionData.fullName = s.fullName;
                sessionData.email = s.email;
                sessionData.balance = s.balance;

                AppState::instance().setSession(sessionData);
                m_dashboardPage->updateBalance(s.balance, s.fullName);

                stackedWidget->setCurrentIndex(1); // Go to Dashboard
                dialog->close();
            }
            else {
                QMessageBox::warning(dialog, "Login Failed", "Invalid credentials");
            }
        });

    // Handle Signup
    connect(authManager, &AuthManager::signupFailed, dialog, [dialog](const QString& message) {
        QMessageBox::warning(dialog, "Signup Failed", "Database Error:\n" + message);
    });

    connect(dialog, &LoginDialog::signupRequested, this,
        [this, dialog](const QString& username, const QString& email, const QString& pass, const QString& fullName, const QString& phone) {

            QString defaultRole = "user"; // Standard user role

            // Step 1: Attempt to create the user in DB
            if (authManager->signup(username, email, pass, fullName, phone, defaultRole)) {

                // Step 2: Auto-login after successful creation
                if (authManager->login(email, pass, defaultRole)) {
                    auto& s = authManager->currentSession();
                    SessionData sessionData;
                    sessionData.userId = QString::number(s.userId);
                    sessionData.fullName = s.fullName;
                    sessionData.email = s.email;
                    sessionData.balance = 0.0; // New accounts have 0 balance

                    AppState::instance().setSession(sessionData);
                    m_dashboardPage->updateBalance(0.0, s.fullName);

                    stackedWidget->setCurrentIndex(1); // Go to Dashboard
                    dialog->close();
                } else {
                    QMessageBox::warning(dialog, "Auto-Login Failed", "Account created, but automatic login failed.");
                }
            }
        });

    dialog->show();
}

void AppController::switchTheme(bool /*dark*/)
{
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
        QLineEdit, QDoubleSpinBox, QComboBox {
            background-color: #0f0f12;
            color: #ffffff;
            border: 1px solid #2a2a35;
            border-radius: 8px;
            padding: 8px;
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

void AppController::closeEvent(QCloseEvent* event)
{
    QSettings settings("CalxSecure", "CalxSecureApp");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    QMainWindow::closeEvent(event);
}
