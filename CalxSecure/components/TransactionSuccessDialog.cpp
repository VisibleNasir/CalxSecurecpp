// TransactionSuccessDialog.cpp
#include "TransactionSuccessDialog.h"
#include <QApplication>
#include <QScreen>
#include <QFont>
#include <QRandomGenerator>
#include <QPainter>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

TransactionSuccessDialog::TransactionSuccessDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(380, 520);
    setupUI();
    createConfettiParticles();

    confettiTimer = new QTimer(this);
    connect(confettiTimer, &QTimer::timeout, this, [this]() {
        update();
        confettiCount++;
        if (confettiCount > 120) confettiTimer->stop();
        });
}

void TransactionSuccessDialog::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 40, 30, 40);
    mainLayout->setSpacing(20);

    iconLabel = new QLabel(this);
    iconLabel->setFixedSize(120, 120);
    iconLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(iconLabel, 0, Qt::AlignHCenter);

    titleLabel = new QLabel(this);
    titleLabel->setFont(QFont("Segoe UI", 24, QFont::Bold));
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #22C55E;");
    mainLayout->addWidget(titleLabel);

    amountLabel = new QLabel(this);
    amountLabel->setFont(QFont("Segoe UI", 28, QFont::Bold));
    amountLabel->setAlignment(Qt::AlignCenter);
    amountLabel->setStyleSheet("color: white;");
    mainLayout->addWidget(amountLabel);

    detailsLabel = new QLabel(this);
    detailsLabel->setFont(QFont("Segoe UI", 14));
    detailsLabel->setAlignment(Qt::AlignCenter);
    detailsLabel->setWordWrap(true);
    detailsLabel->setStyleSheet("color: #9CA3AF;");
    mainLayout->addWidget(detailsLabel);

    mainLayout->addStretch();

    continueButton = new QPushButton("Continue", this);
    continueButton->setFixedHeight(56);
    continueButton->setFont(QFont("Segoe UI", 16, QFont::Bold));
    continueButton->setCursor(Qt::PointingHandCursor);
    continueButton->setStyleSheet(R"(
        QPushButton {
            background-color: #22C55E;
            color: black;
            border: none;
            border-radius: 12px;
        }
        QPushButton:hover { background-color: #16A34A; }
        QPushButton:pressed { background-color: #15803D; }
    )");
    mainLayout->addWidget(continueButton);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(30);
    shadow->setColor(QColor(0, 0, 0, 160));
    shadow->setOffset(0, 10);
    setGraphicsEffect(shadow);

    connect(continueButton, &QPushButton::clicked, this, &TransactionSuccessDialog::onContinueClicked);
}

void TransactionSuccessDialog::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect rect = this->rect().adjusted(10, 10, -10, -10);
    painter.setBrush(QColor(17, 17, 17));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect, 24, 24);

    for (const auto& p : particles) {
        painter.save();
        painter.translate(p.pos.x(), p.pos.y());
        painter.rotate(p.rotation);
        painter.setBrush(p.color);
        painter.setPen(Qt::NoPen);
        painter.drawRect(QRectF(-p.size / 2, -p.size / 2, p.size, p.size * 0.6));
        painter.restore();
    }
}

void TransactionSuccessDialog::createConfettiParticles()
{
    particles.clear();
    for (int i = 0; i < 80; ++i) {
        ConfettiParticle p;
        p.pos = QPointF(QRandomGenerator::global()->bounded(width()), -20.0);
        int hue = QRandomGenerator::global()->bounded(80, 160);
        p.color = QColor::fromHsv(hue, 200, 255);
        p.size = QRandomGenerator::global()->bounded(6, 14);
        p.rotation = QRandomGenerator::global()->bounded(360);
        particles.append(p);
    }
}

void TransactionSuccessDialog::startConfetti()
{
    confettiCount = 0;
    confettiTimer->start(33);
}

void TransactionSuccessDialog::showSuccess(TransactionType type, const QString& amount, const QString& details)
{
    // Icon
    QPixmap pix(120, 120);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QColor(22, 197, 94));
    p.setPen(Qt::NoPen);
    p.drawEllipse(10, 10, 100, 100);
    p.setPen(QPen(Qt::black, 12, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawPolyline(QVector<QPoint>{QPoint(35, 60), QPoint(50, 78), QPoint(85, 35)});
    iconLabel->setPixmap(pix);

    // Text
    switch (type) {
    case Payment:  titleLabel->setText("Payment Successful"); break;
    case Debit:    titleLabel->setText("Debited Successfully"); break;
    case Credit:   titleLabel->setText("Credited Successfully"); break;
    case Recharge: titleLabel->setText("Recharge Done"); break;
    }

    amountLabel->setText(amount);
    detailsLabel->setText(details.isEmpty()
        ? "TXN" + QString::number(QRandomGenerator::global()->bounded(100000, 999999))
        : details);

    show();
    QRect screen = QApplication::primaryScreen()->geometry();
    move((screen.width() - width()) / 2, (screen.height() - height()) / 2);

    createConfettiParticles();
    startConfetti();

    QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity");
    anim->setDuration(400);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void TransactionSuccessDialog::onContinueClicked()
{
    QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity");
    anim->setDuration(250);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    connect(anim, &QPropertyAnimation::finished, this, &TransactionSuccessDialog::accept);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}