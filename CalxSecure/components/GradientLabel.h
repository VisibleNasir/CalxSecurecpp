#pragma once
#include <QLabel>

class GradientLabel : public QLabel {
    Q_OBJECT
public:
    explicit GradientLabel(const QString& text, QWidget* parent = nullptr);
};