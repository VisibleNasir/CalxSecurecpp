#pragma once

#include <QWidget>
#include "ui_LoginDialog.h"

class LoginDialog : public QWidget
{
	Q_OBJECT

public:
	LoginDialog(QWidget *parent = nullptr);
	~LoginDialog();
    bool isSignupMode() const { return m_isSignup; }

signals:
    void loginRequested(const QString& email, const QString& password, const QString& role);
    void signupRequested(const QString& username, const QString& email,
        const QString& password, const QString& fullName, const QString& phone);
private slots:
    void onToggleClicked();
    void onSubmitClicked();

private:
	Ui::LoginDialogClass ui;
    bool m_isSignup = false;

    void updateUIForMode();
};

