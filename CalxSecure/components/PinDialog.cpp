#include "PinDialog.h"
#include <QKeyEvent>
#include <QRegularExpression>

PinDialog::PinDialog(bool isSetup, QWidget* parent)
    : QWidget(parent), m_isSetup(isSetup)
{
    ui.setupUi(this);

    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    auto setupBox = [](QLineEdit* box) {
        box->setMaxLength(1);
        box->setAlignment(Qt::AlignCenter);
        box->setEchoMode(QLineEdit::Password);
        };
    setupBox(ui.d1);
    setupBox(ui.d2);
    setupBox(ui.d3);
    setupBox(ui.d4);

    // Auto focus next
    connect(ui.d1, &QLineEdit::textChanged, this, [=](const QString& t) { if (!t.isEmpty()) ui.d2->setFocus(); });
    connect(ui.d2, &QLineEdit::textChanged, this, [=](const QString& t) { if (!t.isEmpty()) ui.d3->setFocus(); });
    connect(ui.d3, &QLineEdit::textChanged, this, [=](const QString& t) { if (!t.isEmpty()) ui.d4->setFocus(); });

    // Backspace handling
    ui.d1->installEventFilter(this);
    ui.d2->installEventFilter(this);
    ui.d3->installEventFilter(this);
    ui.d4->installEventFilter(this);

    if (m_isSetup) {
        ui.title->setText("Create Transaction PIN");
        ui.btnSubmit->setText("Create PIN");
    }
    else {
        ui.title->setText("Verify Transaction PIN");
        ui.btnSubmit->setText("Verify");
    }

    connect(ui.btnSubmit, &QPushButton::clicked, this, &PinDialog::onSubmit);
    connect(ui.btnCancel, &QPushButton::clicked, this, &PinDialog::close);  // Safe close
}

PinDialog::~PinDialog() {}

bool PinDialog::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        if (key->key() == Qt::Key_Backspace) {
            QLineEdit* current = qobject_cast<QLineEdit*>(obj);
            if (current && current->text().isEmpty()) {
                if (obj == ui.d2) { ui.d1->clear(); ui.d1->setFocus(); }
                else if (obj == ui.d3) { ui.d2->clear(); ui.d2->setFocus(); }
                else if (obj == ui.d4) { ui.d3->clear(); ui.d3->setFocus(); }
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void PinDialog::onSubmit()
{
    QString pin = ui.d1->text() + ui.d2->text() + ui.d3->text() + ui.d4->text();

    if (!QRegularExpression("^\\d{4}$").match(pin).hasMatch()) {
        ui.lblError->setText("Enter valid 4-digit PIN");
        ui.lblError->setVisible(true);
        return;
    }

    if (m_isSetup) {
        emit pinCreated(pin);
    }
    else {
        emit pinEntered(pin);
    }

    close();
}