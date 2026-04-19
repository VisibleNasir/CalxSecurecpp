// components/GradientLabel.cpp
#include "GradientLabel.h"

GradientLabel::GradientLabel(const QString& text, QWidget* parent) : QLabel(text, parent) {
    setStyleSheet(R"(
        QLabel {
            font-size: 42px;
            font-weight: 800;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                        stop:0 #00d4ff, stop:1 #7b00ff);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            padding: 10px;
        }
    )");
    setAlignment(Qt::AlignCenter);
}