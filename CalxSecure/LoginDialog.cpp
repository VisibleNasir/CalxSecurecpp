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
	QRegularExpression regex(R"((\w+)(.\w+)*@(\w+)(.\w+)+)");
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
	resetForm(); // ✅ Clear everything
	updateUIForMode(); // ✅ Update UI text
}
// ==================== SUBMIT HANDLER ====================
void LoginDialog::onSubmitClicked()
{
	QString email = ui.leEmail->text().trimmed();
	QString password = ui.lePassword->text();

	qDebug() << "[AUTH] Submit Clicked! Mode:" << (m_isSignup ? "Signup" : "Login");
	qDebug() << "[AUTH] Email entered:" << email;

	bool isValid = true;
	QString errorMessage = "";

	// Reset errors
	ui.lblEmailError->setVisible(false);
	ui.lblPasswordError->setVisible(false);
	ui.leEmail->setStyleSheet("");
	ui.lePassword->setStyleSheet("");
	// ==================== EMAIL VALIDATION ====================
	if (email.isEmpty()) {
		errorMessage += "- Email is required.\n";
		ui.leEmail->setStyleSheet("border:1px solid #ff4d4f;");
		isValid = false;
	}
	else if (!isValidEmail(email)) {
		errorMessage += "- Enter a valid email format (e.g., name@domain.com).\n";
		ui.leEmail->setStyleSheet("border:1px solid #ff4d4f;");
		isValid = false;
	}
	// ==================== PASSWORD VALIDATION ====================
	if (password.isEmpty()) {
		errorMessage += "- Password is required.\n";
		ui.lePassword->setStyleSheet("border:1px solid #ff4d4f;");
		isValid = false;
	}
	else if (password.length() < 8) {
		errorMessage += "- Password must be exactly 8 characters or more.\n";
		ui.lePassword->setStyleSheet("border:1px solid #ff4d4f;");
		isValid = false;
	}
	// If validation fails, SHOW A POPUP so it doesn't fail silently
	if (!isValid) {
		qWarning() << "[AUTH] Validation Failed:" << errorMessage;
		QMessageBox::warning(this, "Validation Error", errorMessage);
		return; 
	}
	// ==================== PROCEED TO ROUTER ====================
	if (m_isSignup) {
		qDebug() << "[AUTH] Emitting signupRequested...";
		QString baseName = email.section('@', 0, 0);
		QString uniqueSuffix = QString::number(QDateTime::currentMSecsSinceEpoch() % 10000);
		QString username = baseName + uniqueSuffix;
		
		emit signupRequested(username, email, password, "New User", "+91 0000000000");
	}
	else {
		// Ensure role exists and is lowercase before passing
		QString role = "user"; // Fallback default
		if (ui.cbRole) {
			role = ui.cbRole->currentText().toLower();
		}
		
		qDebug() << "[AUTH] Emitting loginRequested with Role:" << role;
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
	resetForm(); // ✅ Clear everything
	updateUIForMode();
}