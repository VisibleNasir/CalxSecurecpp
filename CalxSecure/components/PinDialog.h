#pragma once
#include <QWidget>
#include "ui_PinDialog.h"

class PinDialog : public QWidget
{
    Q_OBJECT

public:
    explicit PinDialog(bool isSetup = false, QWidget* parent = nullptr);
    ~PinDialog();

signals:
    void pinEntered(const QString& pin);
    void pinCreated(const QString& pin);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onSubmit();

private:
    Ui::PinDialogClass ui;
    bool m_isSetup;
};