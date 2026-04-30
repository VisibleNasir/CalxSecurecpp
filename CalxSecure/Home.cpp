#include "Home.h"
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QEasingCurve>
#include <QTimer>

Home::Home(QWidget* parent) : QWidget(parent)
{
    ui.setupUi(this);

    // Connect button
    connect(ui.primaryBtn, &QPushButton::clicked,
        this, &Home::onPrimaryBtnClicked);

    // Setup entrance animations
    QTimer::singleShot(100, this, &Home::setupAnimations);
}

Home::~Home()
{}

void Home::setupAnimations()
{
    // Hero Title Animation
    QPropertyAnimation* titleAnim = new QPropertyAnimation(ui.heroTitle, "pos", this);
    titleAnim->setDuration(800);
    titleAnim->setStartValue(QPoint(ui.heroTitle->x(), ui.heroTitle->y() - 60));
    titleAnim->setEndValue(ui.heroTitle->pos());
    titleAnim->setEasingCurve(QEasingCurve::OutExpo);

    // Subtitle Animation
    QPropertyAnimation* subtitleAnim = new QPropertyAnimation(ui.heroSubtitle, "pos", this);
    subtitleAnim->setDuration(900);
    subtitleAnim->setStartValue(QPoint(ui.heroSubtitle->x(), ui.heroSubtitle->y() - 40));
    subtitleAnim->setEndValue(ui.heroSubtitle->pos());
    subtitleAnim->setEasingCurve(QEasingCurve::OutExpo);

    // Button Animation
    QPropertyAnimation* btnAnim = new QPropertyAnimation(ui.primaryBtn, "geometry", this);
    btnAnim->setDuration(1000);
    QRect startRect = ui.primaryBtn->geometry();
    startRect.moveTop(startRect.top() + 40);
    btnAnim->setStartValue(startRect);
    btnAnim->setEndValue(ui.primaryBtn->geometry());
    btnAnim->setEasingCurve(QEasingCurve::OutBack);

    // Cards fade-in + slide-up
    QParallelAnimationGroup* cardsGroup = new QParallelAnimationGroup(this);

    for (int i = 1; i <= 3; ++i) {
        QFrame* card = findChild<QFrame*>(QString("card%1").arg(i));
        if (!card) continue;

        QPropertyAnimation* cardAnim = new QPropertyAnimation(card, "pos", this);
        cardAnim->setDuration(700 + i * 100);
        cardAnim->setStartValue(QPoint(card->x(), card->y() + 80));
        cardAnim->setEndValue(card->pos());
        cardAnim->setEasingCurve(QEasingCurve::OutCubic);

        cardsGroup->addAnimation(cardAnim);
    }

    // Sequential group
    QSequentialAnimationGroup* group = new QSequentialAnimationGroup(this);
    group->addAnimation(titleAnim);
    group->addAnimation(subtitleAnim);
    group->addAnimation(btnAnim);
    group->addPause(200);
    group->addAnimation(cardsGroup);

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void Home::onPrimaryBtnClicked()
{
    // Optional: Add click animation
    QPropertyAnimation* scaleAnim = new QPropertyAnimation(ui.primaryBtn, "geometry", this);
    scaleAnim->setDuration(150);
    QRect orig = ui.primaryBtn->geometry();
    scaleAnim->setStartValue(orig);
    scaleAnim->setEndValue(orig.adjusted(-4, -4, 4, 4));
    scaleAnim->setEasingCurve(QEasingCurve::OutQuad);

    connect(scaleAnim, &QPropertyAnimation::finished, this, [this]() {
        emit dashboardRequested();
        });

    scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);
}