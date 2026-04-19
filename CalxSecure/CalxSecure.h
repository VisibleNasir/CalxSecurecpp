#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_CalxSecure.h"

class CalxSecure : public QMainWindow
{
    Q_OBJECT

public:
    CalxSecure(QWidget *parent = nullptr);
    ~CalxSecure();

private:
    Ui::CalxSecureClass ui;
};

