#include "LoginDialog.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QDateTime>
#include <QCloseEvent>
// ==================== CONSTRUCTOR ====================
LoginDialog::LoginDialog(QWidget* parent)
: QWidget(parent)
{
ui.setupUi(this);
// Make it look like a proper dialog
setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
setAttribute(Qt::WA_TranslucentBackground);
// ==================== STYLE FIX (LESS PADDING) ====================
ui.lblEmailError->setStyleSheet(
"color:#ff4d4f;"
"font-size:11px;"
"padding:2px 0px;"
);
ui.lblPasswordError->setStyleSheet(
"color:#ff4d4f;"
"font-size:11px;"
"padding:2px 0px;"
);
// ==================== LIVE VALIDATION RESET ====================
connect(ui.leEmail, &QLineEdit::textChanged, this, [this] {
ui.lblEmailError->setVisible(false);
ui.leEmail->setStyleSheet("");
});
connect(ui.lePassword, &QLineEdit::textChanged, this, [this] {
ui.lblPasswordError->setVisible(false);
ui.lePassword->setStyleSheet("");
});
// ==================== SIGNALS ====================
connect(ui.btnSubmit, &QPushButton::clicked, this, &LoginDialog::onSubmitClicked);
connect(ui.lblToggle, &QLabel::linkActivated, this, &LoginDialog::onToggleClicked);
// Toggle label setup
ui.lblToggle->setTextFormat(Qt::RichText);
ui.lblToggle->setOpenExternalLinks(false);
updateUIForMode();
}
// ==================== DESTRUCTOR ====================
LoginDialog::~LoginDialog() {}
// ==================== HELPER: EMAIL VALIDATION ====================
bool isValidEmail(const QString& email)
{
QRegularExpression regex(R"((\w+)(\.\w+)*@(\w+)(\.\w+)+)");
return regex.match(email).hasMatch();
}
// ==================== RESET FORM ====================
void LoginDialog::resetForm()
{
ui.leEmail->clear();
ui.lePassword->clear();
ui.lblEmailError->clear();
ui.lblPasswordError->clear();
ui.lblEmailError->setVisible(false);
ui.lblPasswordError->setVisible(false);
ui.leEmail->setStyleSheet("");
ui.lePassword->setStyleSheet("");
}
// ==================== UI MODE SWITCH ====================
void LoginDialog::updateUIForMode()
{
if (m_isSignup) {
ui.title->setText("Create Account");
ui.subtitle->setText("Join CalxSecure today");
ui.btnSubmit->setText("Create Account");
ui.lblToggle->setText(
"<a href=\"#\" style=\"color:#888888; text-decoration:none;\">"
"Already have an account? Sign In"
);
}
else {
ui.title->setText("Welcome Back");
ui.subtitle->setText("Sign in to continue to CalxSecure");
ui.btnSubmit->setText("Sign In");
ui.lblToggle->setText(
"<a href=\"#\" style=\"color:#888888; text-decoration:none;\">"
"Don't have an account? Sign Up"
);
}
}
// ==================== TOGGLE LOGIN <-> SIGNUP ====================
void LoginDialog::onToggleClicked()
{
m_isSignup = !m_isSignup;
resetForm();
updateUIForMode();
}
// ==================== SUBMIT HANDLER ====================
void LoginDialog::onSubmitClicked()
{
QString email = ui.leEmail->text().trimmed();
QString password = ui.lePassword->text();

bool isValid = true;
QString errorMessage = "";

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
errorMessage += "- Email is required.\n";
isValid = false;
}
else if (!isValidEmail(email)) {
ui.lblEmailError->setText("Enter a valid email address");
ui.lblEmailError->setVisible(true);
ui.leEmail->setStyleSheet("border:1px solid #ff4d4f;");
errorMessage += "- Enter a valid email format (e.g., name@domain.com).\n";
isValid = false;
}

// Password validation
if (password.isEmpty()) {
ui.lblPasswordError->setText("Password is required");
ui.lblPasswordError->setVisible(true);
ui.lePassword->setStyleSheet("border:1px solid #ff4d4f;");
errorMessage += "- Password is required.\n";
isValid = false;
}
else if (password.length() < 8) {
ui.lblPasswordError->setText("Password must be at least 8 characters");
ui.lblPasswordError->setVisible(true);
ui.lePassword->setStyleSheet("border:1px solid #ff4d4f;");
errorMessage += "- Password must be exactly 8 characters or more.\n";
isValid = false;
}

if (!isValid)
return;

// Proceed to router
if (m_isSignup) {
QString baseName = email.section('@', 0, 0);
QString uniqueSuffix = QString::number(QDateTime::currentMSecsSinceEpoch() % 100000);
QString username = baseName + uniqueSuffix;

emit signupRequested(username, email, password, "New User", "+91 0000000000");
}
else {
// Ensure role exists and is lowercase before passing
QString role = "user"; // Fallback default
if (ui.cbRole) {
role = ui.cbRole->currentText().toLower();
}
emit loginRequested(email, password, role);
}
}
// ==================== PREVENT WINDOW CLOSE ====================
void LoginDialog::closeEvent(QCloseEvent* event)
{
event->ignore();
}
// ==================== FORCE LOGIN MODE ====================
void LoginDialog::switchToLoginMode()
{
m_isSignup = false;
resetForm();
updateUIForMode();
}
