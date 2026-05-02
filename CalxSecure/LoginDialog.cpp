#include "LoginDialog.h"
#include <QRegularExpression>
#include <QDateTime>
#include <QApplication>
#include <QKeyEvent>
#include <QCloseEvent>

// ==================== CONSTRUCTOR ====================
LoginDialog::LoginDialog(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    ui.lblEmailError->setStyleSheet("color:#ff4d4f; font-size:11px;");
    ui.lblPasswordError->setStyleSheet("color:#ff4d4f; font-size:11px;");


    // Live validation reset
    connect(ui.leEmail, &QLineEdit::textChanged, this, [this] {
        ui.lblEmailError->setVisible(false);
        });

    connect(ui.lePassword, &QLineEdit::textChanged, this, [this] {
        ui.lblPasswordError->setVisible(false);
        });

    // Signals
    connect(ui.btnSubmit, &QPushButton::clicked, this, &LoginDialog::onSubmitClicked);
    connect(ui.lblToggle, &QLabel::linkActivated, this, &LoginDialog::onToggleClicked);

    updateUIForMode();
}

// ==================== DESTRUCTOR ====================
LoginDialog::~LoginDialog()
{
}

// ==================== VALIDATION ====================
bool isValidEmail(const QString& email)
{
    QRegularExpression regex(R"((\w+)(\.\w+)*@(\w+)(\.\w+)+)");
    return regex.match(email).hasMatch();
}

// ==================== ERROR DISPLAY ====================
void LoginDialog::showError(const QString& msg)
{
    ui.lblEmailError->setStyleSheet("color:#ff4d4f;");
    ui.lblEmailError->setText(msg);
    ui.lblEmailError->setVisible(true);
}

void LoginDialog::showSuccess(const QString& msg)
{
    ui.lblEmailError->setStyleSheet("color:#00c853;");
    ui.lblEmailError->setText(msg);
    ui.lblEmailError->setVisible(true);
}

// ==================== SUBMIT ====================
void LoginDialog::onSubmitClicked()
{
    QString email = ui.leEmail->text().trimmed();
    QString password = ui.lePassword->text();

    bool valid = true;

    ui.lblEmailError->setVisible(false);
    ui.lblPasswordError->setVisible(false);

    if (email.isEmpty() || !isValidEmail(email)) {
        ui.lblEmailError->setText("Enter valid email");
        ui.lblEmailError->setVisible(true);
        valid = false;
    }

    if (password.length() < 8) {
        ui.lblPasswordError->setText("Min 8 characters required");
        ui.lblPasswordError->setVisible(true);
        valid = false;
    }

    if (!valid) return;

    // Temporarily disable the button
    ui.btnSubmit->setEnabled(false);

    if (m_isSignup) {
        ui.btnSubmit->setText("Signing up...");
        QCoreApplication::processEvents(); // Force UI update

        QString username = email.section('@', 0, 0);
        emit signupRequested(username, email, password, "User", "");
    }
    else {
        ui.btnSubmit->setText("Logging in...");
        QCoreApplication::processEvents(); // Force UI update

        emit loginRequested(email, password, "user");
    }
}

// Ensure resetting logic is properly placed so we can call it when failure occurs
void LoginDialog::resetForm()
{
    ui.btnSubmit->setEnabled(true);
    updateUIForMode();
}

// ==================== TOGGLE ====================
void LoginDialog::onToggleClicked()
{
    m_isSignup = !m_isSignup;
    updateUIForMode();
}

// ==================== MODE UI ====================
void LoginDialog::updateUIForMode()
{
    if (m_isSignup) {
        ui.title->setText("Create Account");
        ui.subtitle->setText("Sign up to create your secure account");
        ui.btnSubmit->setText("Sign Up");
        ui.lblToggle->setText("Already have an account? <a href=\"#\" style=\"color:#ffffff;\">Sign In</a>");
    }
    else {
        ui.title->setText("Welcome Back");
        ui.subtitle->setText("Sign in to access your secure account");
        ui.btnSubmit->setText("Sign In");
        ui.lblToggle->setText("Don't have an account? <a href=\"#\" style=\"color:#ffffff;\">Sign Up</a>");
    }
}

// ==================== REQUIRED (FIXES LINKER ERROR) ====================
void LoginDialog::switchToLoginMode()
{
    m_isSignup = false;
    updateUIForMode();
}

// ==================== OPTIONAL CLOSE HANDLING ====================
void LoginDialog::closeEvent(QCloseEvent* event)
{
    // Allow closing (important fix)
    event->accept();
}

// ==================== ESC CLOSE ====================
void LoginDialog::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
        qApp->quit();
}