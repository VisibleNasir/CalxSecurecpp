#pragma once
#include <QFrame>
#include <QVBoxLayout>
#include <QLabel>
#include <QGraphicsDropShadowEffect>

class Card : public QFrame {
    Q_OBJECT
public:
    explicit Card(QWidget* parent = nullptr, bool elevated = true);
    void setTitle(const QString& title);
    void setDescription(const QString& desc);
    QVBoxLayout* contentLayout();

private:
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QLabel* m_descLabel;
};