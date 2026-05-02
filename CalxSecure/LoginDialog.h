#pragma once

#include <QWidget>
#include <QCloseEvent>
#include <QSettings>
#include "ui_LoginDialog.h"

class LoginDialog : public QWidget
{
	Q_OBJECT

public:
	explicit LoginDialog(QWidget *parent = nullptr);
	~LoginDialog();
    bool isSignupMode() const { return m_isSignup; }

    // Public method so AppController can call it to toggle modes
    void switchToLoginMode();
    void resetForm();
    void showError(const QString& msg);
	void showSuccess(const QString& msg);
    void keyPressEvent(QKeyEvent* event) override;
protected:
    // Override close event to prevent users from bypassing login
    void closeEvent(QCloseEvent *event) override;

signals:
    void loginRequested(const QString& email, const QString& password, const QString& role);
    void signupRequested(const QString& username, const QString& email,
        const QString& password, const QString& fullName,
        const QString& phone);

private slots:
    void onToggleClicked();
    void onSubmitClicked();

private:
	Ui::LoginDialogClass ui;
    bool m_isSignup = false;

    void updateUIForMode();
};

