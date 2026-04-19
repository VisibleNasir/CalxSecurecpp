// navigation/DockWidget.h
#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QStackedWidget>

class DockWidget : public QWidget {
    Q_OBJECT
public:
    explicit DockWidget(QStackedWidget* stacked, QWidget* parent = nullptr);

signals:
    void pageRequested(int index);

private:
    void createDockButton(const QString& iconText, const QString& tooltip, int pageIndex);

    QVBoxLayout* m_layout;
    QStackedWidget* m_stackedWidget;
};