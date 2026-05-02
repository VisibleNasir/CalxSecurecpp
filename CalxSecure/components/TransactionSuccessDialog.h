// TransactionSuccessDialog.h
#ifndef TRANSACTIONSUCCESSDIALOG_H
#define TRANSACTIONSUCCESSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QTimer>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>

class TransactionSuccessDialog : public QDialog
{
    Q_OBJECT

public:
    enum TransactionType {
        Payment,
        Debit,
        Credit,
        Recharge
    };

    explicit TransactionSuccessDialog(QWidget* parent = nullptr);
    void showSuccess(TransactionType type, const QString& amount, const QString& details = "");

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onContinueClicked();
    void startConfetti();

private:
    void setupUI();
    void createConfettiParticles();

    QLabel* iconLabel;
    QLabel* titleLabel;
    QLabel* amountLabel;
    QLabel* detailsLabel;
    QPushButton* continueButton;

    QVBoxLayout* mainLayout;

    // Confetti
    struct ConfettiParticle {
        QPointF pos;
        QPointF velocity;
        QColor color;
        float size;
        float rotation;
        float rotationSpeed;
    };
    QVector<ConfettiParticle> particles;
    QTimer* confettiTimer;
    int confettiCount = 0;
};

#endif // TRANSACTIONSUCCESSDIALOG_H