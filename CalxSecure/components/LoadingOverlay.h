#pragma once
#include <QWidget>
#include <QLabel>
#include <QMovie> // for spinner if you want

class LoadingOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit LoadingOverlay(QWidget* parent = nullptr);
    void showLoading(const QString& text = "CalxSecure...");
    void hideLoading();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QLabel* m_label;
    // QMovie* m_spinner; // optional
};