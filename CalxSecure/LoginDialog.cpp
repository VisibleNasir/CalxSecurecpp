#include "LoginDialog.h"
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    // Make it look like a proper dialog
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // Connect signals
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

    if (email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill all required fields");
        return;
    }

    if (m_isSignup) {
        // For full signup (you can extend the UI later with more fields)
        emit signupRequested("defaultuser", email, password, "New User", "+91 0000000000");
    }
    else {
        QString role = ui.cbRole->currentText().toLower();
        emit loginRequested(email, password, role);
    }

    // Do NOT call accept() here — let AppController handle success/failure
}