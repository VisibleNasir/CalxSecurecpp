// config/GlobalStyle.cpp
#include "GlobalStyle.h"

QString GlobalStyle::getDarkTheme() {
    return R"(
        * {
            font-family: 'Segoe UI', system-ui, -apple-system, BlinkMacSystemFont;
            font-size: 14px;
        }
        QMainWindow, QDialog, QWidget {
            background-color: #0f0f12;
            color: #e0e0e0;
        }
        QFrame#Card {
            background-color: #1a1a1f;
            border: 1px solid #2a2a32;
            border-radius: 16px;
        }
        QPushButton {
            padding: 12px 24px;
            border-radius: 12px;
            font-weight: 600;
        }
        QLineEdit, QComboBox, QSpinBox {
            background-color: #1f1f26;
            border: 1px solid #3a3a44;
            border-radius: 10px;
            padding: 12px;
            selection-background-color: #00b4d8;
        }
        QLineEdit:focus, QComboBox:focus {
            border: 1px solid #00b4d8;
        }
        QLabel {
            color: #e0e0e0;
        }
    )";
}

QString GlobalStyle::getLightTheme() {
    return R"(
        * { font-family: 'Segoe UI', system-ui; font-size: 14px; }
        QMainWindow, QDialog, QWidget { background-color: #f8f9fa; color: #212529; }
        QFrame#Card {
            background-color: #ffffff;
            border: 1px solid #e5e7eb;
            border-radius: 16px;
        }
    )";
}

QString GlobalStyle::getCardStyle() {
    return "QFrame#Card { background-color: #1a1a1f; border: 1px solid #2a2a32; "
        "border-radius: 16px; padding: 20px; }";
}

QString GlobalStyle::getButtonPrimaryStyle() {
    return "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "stop:0 #00b4d8, stop:1 #0077b6); color: white; border: none; "
        "border-radius: 12px; font-weight: 600; padding: 12px 28px; } "
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "stop:0 #0099c4, stop:1 #005f99); }";
}