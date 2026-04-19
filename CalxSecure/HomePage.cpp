// pages/HomePage.cpp
#include "HomePage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "components/AnimatedButton.h"
#include <QLabel>
#include "components/Card.h"

HomePage::HomePage(QWidget* parent) : QWidget(parent)
{
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_contentWidget = new QWidget();
    auto* mainLayout = new QVBoxLayout(m_contentWidget);
    mainLayout->setSpacing(60);
    mainLayout->setContentsMargins(40, 40, 40, 80);

    setupHero();
    setupSecuritySection();
    setupTeamSection();
    setupCTA();
    setupFooter();

    m_scrollArea->setWidget(m_contentWidget);
    auto* outer = new QVBoxLayout(this);
    outer->addWidget(m_scrollArea);
}

void HomePage::setupHero() {
    auto* hero = new QWidget();
    auto* layout = new QVBoxLayout(hero);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(24);

    auto* title = new GradientLabel("Send, Spend, Grow — with confidence", this);
    title->setWordWrap(true);
    title->setAlignment(Qt::AlignCenter);

    auto* subtitle = new QLabel("Secure • Fast • Private\nThe next-generation payment gateway", this);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("font-size: 20px; color: #a0a0b0;");

    auto* btnGetStarted = new AnimatedButton("Get Started Securely", this);
    btnGetStarted->setFixedWidth(280);

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(20);
    layout->addWidget(btnGetStarted, 0, Qt::AlignCenter);

    m_contentWidget->layout()->addWidget(hero);
}

void HomePage::setupSecuritySection() {
    auto* section = new QWidget();
    auto* title = new QLabel("Bank-grade Security", this);
    title->setStyleSheet("font-size: 28px; font-weight: 700;");

    auto* grid = new QHBoxLayout();

    Card* c1 = new Card();
    c1->setTitle("AES-256 Encryption");
    c1->setDescription("Every transaction is encrypted end-to-end.");

    Card* c2 = new Card();
    c2->setTitle("AI Fraud Detection");
    c2->setDescription("Real-time risk scoring and anomaly detection.");

    Card* c3 = new Card();
    c3->setTitle("Multi-Factor Auth");
    c3->setDescription("Biometric + device binding protection.");

    grid->addWidget(c1);
    grid->addWidget(c2);
    grid->addWidget(c3);

    auto* v = new QVBoxLayout(section);
    v->addWidget(title);
    v->addLayout(grid);

    m_contentWidget->layout()->addWidget(section);
}

void HomePage::setupTeamSection() {
    auto* section = new QWidget();
    auto* title = new QLabel("Built by Visionaries", this);
    title->setStyleSheet("font-size: 28px; font-weight: 700; text-align: center;");

    auto* grid = new QHBoxLayout();

    QStringList names = { "Roni Bhakta", "Nasir Nadaf", "Tanishq Dasari", "Vikas Budhyal" };
    QStringList roles = { "Founder & CEO", "Lead Architect", "Product Designer", "Security Engineer" };

    /*for (int i = 0; i < 4; ++i) {
        Card* card = new Card(false);
        card->setTitle(names[i]);
        card->setDescription(roles[i]);
        grid->addWidget(card);
    }*/

    auto* v = new QVBoxLayout(section);
    v->addWidget(title);
    v->addLayout(grid);
    m_contentWidget->layout()->addWidget(section);
}

void HomePage::setupCTA() {
    auto* cta = new QWidget();
    auto* layout = new QVBoxLayout(cta);
    layout->setAlignment(Qt::AlignCenter);

    auto* label = new GradientLabel("Ready to experience secure payments?", this);
    auto* btn = new AnimatedButton("Launch CalxSecure Now", this);
    btn->setFixedWidth(320);

    layout->addWidget(label);
    layout->addWidget(btn);
    m_contentWidget->layout()->addWidget(cta);
}

void HomePage::setupFooter() {
    auto* footer = new QLabel("© 2026 CalxSecure • Built with ❤️ in India • All Rights Reserved", this);
    footer->setAlignment(Qt::AlignCenter);
    footer->setStyleSheet("color: #666; padding: 40px;");
    m_contentWidget->layout()->addWidget(footer);
}