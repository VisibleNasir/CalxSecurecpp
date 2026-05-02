#include "LoadingOverlay.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

LoadingOverlay::LoadingOverlay(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    m_label = new QLabel(this);
    m_label->setStyleSheet("QLabel {"
        "color: #ffffff; "
        "font-size: 32px; "
        "font-weight: 600; "
        "letter-spacing: 2px;"
        "}");
    m_label->setAlignment(Qt::AlignCenter);

    layout->addWidget(m_label);
    setVisible(false);
}

void LoadingOverlay::showLoading(const QString& text)
{
    m_label->setText(text);
    setVisible(true);
    raise(); // bring to front

    // Optional fade-in
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);
    QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity");
    anim->setDuration(300);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void LoadingOverlay::hideLoading()
{
    QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
    if (effect) {
        QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity");
        anim->setDuration(300);
        anim->setStartValue(1.0);
        anim->setEndValue(0.0);
        connect(anim, &QPropertyAnimation::finished, this, [this]() { setVisible(false); });
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
    else {
        setVisible(false);
    }
}

void LoadingOverlay::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (parentWidget())
        setGeometry(parentWidget()->rect());
}