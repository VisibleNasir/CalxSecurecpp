// pages/HomePage.h
#pragma once
#include <QWidget>
#include <QScrollArea>
#include "components/Card.h"
#include "components/GradientLabel.h"

class HomePage : public QWidget {
    Q_OBJECT
public:
    explicit HomePage(QWidget* parent = nullptr);

private:
    void setupHero();
    void setupSecuritySection();
    void setupTeamSection();
    void setupCTA();
    void setupFooter();

    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
};