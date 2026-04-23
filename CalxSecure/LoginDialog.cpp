#include "LoginDialog.h"
#include <QMessageBox>
#include <QRegularExpression>

LoginDialog::LoginDialog(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    // Make it look like a proper dialog
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // Connect signals
    connect(ui.leEmail, &QLineEdit::textChanged, this, [=]() {
        ui.lblEmailError->setVisible(false);
        ui.leEmail->setStyleSheet("");
        });
    connect(ui.btnSubmit, &QPushButton::clicked, this, &LoginDialog::onSubmitClicked);
    connect(ui.lblToggle, &QLabel::linkActivated, this, &LoginDialog::onToggleClicked);

    // Make toggle label clickable and styled
    ui.lblToggle->setTextFormat(Qt::RichText);
    ui.lblToggle->setOpenExternalLinks(false);

    updateUIForMode();
}

LoginDialog::~LoginDialog()
{
}
bool isValidEmail(const QString& email)
{
    QRegularExpression regex(R"((\w+)(\.\w+)*@(\w+)(\.\w+)+)");
    return regex.match(email).hasMatch();
}
void LoginDialog::updateUIForMode()
{
    if (m_isSignup) {
        ui.title->setText("Create Account");
        ui.subtitle->setText("Join CalxSecure today");
        ui.btnSubmit->setText("Create Account");
        ui.lblToggle->setText("<a href=\"#\" style=\"color:#888888; text-decoration:none;\">"
            "Already have an account? <b>Sign In</b></a>");
    }
    else {
        ui.title->setText("Welcome Back");
        ui.subtitle->setText("Sign in to continue to CalxSecure");
        ui.btnSubmit->setText("Sign In");
        ui.lblToggle->setText("<a href=\"#\" style=\"color:#888888; text-decoration:none;\">"
            "Don't have an account? <b>Sign Up</b></a>");
    }
}

void LoginDialog::onToggleClicked()
{
    m_isSignup = !m_isSignup;
    updateUIForMode();
}

void LoginDialog::onSubmitClicked()
{
    QString email = ui.leEmail->text().trimmed();
    QString password = ui.lePassword->text();

    bool isValid = true;

    // Reset errors
    ui.lblEmailError->setVisible(false);
    ui.lblPasswordError->setVisible(false);

    ui.leEmail->setStyleSheet("");
    ui.lePassword->setStyleSheet("");

    // Email validation
    if (email.isEmpty()) {
        ui.lblEmailError->setText("Email is required");
        ui.lblEmailError->setVisible(true);
        ui.leEmail->setStyleSheet("border:1px solid #ff4d4f;");
        isValid = false;
    }
    else if (!isValidEmail(email)) {
        ui.lblEmailError->setText("Enter a valid email address");
        ui.lblEmailError->setVisible(true);
        ui.leEmail->setStyleSheet("border:1px solid #ff4d4f;");
        isValid = false;
    }

    // Password validation
    if (password.isEmpty()) {
        ui.lblPasswordError->setText("Password is required");
        ui.lblPasswordError->setVisible(true);
        ui.lePassword->setStyleSheet("border:1px solid #ff4d4f;");
        isValid = false;
    }
    else if (password.length() < 6) {
        ui.lblPasswordError->setText("Password must be at least 6 characters");
        ui.lblPasswordError->setVisible(true);
        ui.lePassword->setStyleSheet("border:1px solid #ff4d4f;");
        isValid = false;
    }

    if (!isValid)
        return;

    // Proceed
    if (m_isSignup) {
        emit signupRequested("defaultuser", email, password, "New User", "+91 0000000000");
    }
    else {
        QString role = ui.cbRole->currentText().toLower();
        emit loginRequested(email, password, role);
    }
}