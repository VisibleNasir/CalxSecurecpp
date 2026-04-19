// components/AnimatedButton.cpp
#include "AnimatedButton.h"
#include "../../CalxSecure/GlobalStyle.h"

AnimatedButton::AnimatedButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
{
    setStyleSheet(GlobalStyle::getButtonPrimaryStyle());

    m_scaleAnim = new QPropertyAnimation(this, "scale", this);
    m_scaleAnim->setDuration(180);
    m_scaleAnim->setEasingCurve(QEasingCurve::OutCubic);
}

void AnimatedButton::enterEvent(QEnterEvent* event) {
    m_scaleAnim->stop();
    m_scaleAnim->setStartValue(m_scale);
    m_scaleAnim->setEndValue(1.05);
    m_scaleAnim->start();
    QPushButton::enterEvent(event);
}

void AnimatedButton::leaveEvent(QEvent* event) {
    m_scaleAnim->stop();
    m_scaleAnim->setStartValue(m_scale);
    m_scaleAnim->setEndValue(1.0);
    m_scaleAnim->start();
    QPushButton::leaveEvent(event);
}

void AnimatedButton::setScale(qreal s) {
    m_scale = s;
    //setTransform(QTransform().scale(s, s));
}