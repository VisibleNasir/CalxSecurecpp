#pragma once
#include <QPushButton>
#include <QPropertyAnimation>

class AnimatedButton : public QPushButton {
    Q_OBJECT
        Q_PROPERTY(qreal scale READ scale WRITE setScale)

public:
    explicit AnimatedButton(const QString& text, QWidget* parent = nullptr);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    qreal scale() const { return m_scale; }
    void setScale(qreal s);

    QPropertyAnimation* m_scaleAnim;
    qreal m_scale = 1.0;
};