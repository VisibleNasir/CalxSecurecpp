#include "AdminPage.h"
#include "DatabaseManager.h"
#include "core/AppState.h"
#include <QMessageBox>
#include <QDebug>

AdminPage::AdminPage(QWidget* parent) : QWidget(parent)
{
    ui.setupUi(this);
    setupUI();
    setupConnections();
    loadPendingOnramp();
}

AdminPage::~AdminPage() {}

void AdminPage::setupUI()
{
    ui.lblTitle->setText("Admin Panel - Onramp Transaction Approval");
    ui.lblTitle->setStyleSheet("font-size: 32px; font-weight: 700; color: #ffffff;");
}

void AdminPage::setupConnections()
{
    connect(ui.btnRefresh, &QPushButton::clicked, this, &AdminPage::onRefreshClicked);
    connect(ui.btnApprove, &QPushButton::clicked, this, &AdminPage::onApproveSelected);
    connect(ui.btnReject, &QPushButton::clicked, this, &AdminPage::onRejectSelected);
}

void AdminPage::loadPendingOnramp()
{
    ui.listPending->clear();

    QSqlQuery q = DatabaseManager::instance().executeQuery(R"(
        SELECT id, user_id, amount, payment_method, provider, status, created_at
        FROM onramp_transactions 
        WHERE status = 'pending'
        ORDER BY created_at DESC
    )");

    while (q.next()) {
        int id = q.value("id").toInt();
        double amount = q.value("amount").toDouble();
        QString method = q.value("payment_method").toString();
        QString provider = q.value("provider").toString();
        QString time = q.value("created_at").toDateTime().toString("dd MMM hh:mm");

        QString text = QString("ID:%1 | ₹%2 | %3 (%4) | %5")
            .arg(id).arg(amount, 0, 'f', 2).arg(method).arg(provider).arg(time);

        QListWidgetItem* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, id);
        ui.listPending->addItem(item);
    }

    if (ui.listPending->count() == 0) {
        ui.listPending->addItem("No pending onramp transactions.");
    }
}

void AdminPage::onApproveSelected()
{
    QListWidgetItem* item = ui.listPending->currentItem();
    if (!item) return;

    int txnId = item->data(Qt::UserRole).toInt();

    QSqlQuery q = DatabaseManager::instance().executeQuery(
        "UPDATE onramp_transactions SET status = 'success' WHERE id = ?", { txnId });

    if (q.numRowsAffected() > 0) {
        QMessageBox::information(this, "Success", "Transaction Approved");
        loadPendingOnramp();
    }
}

void AdminPage::onRejectSelected()
{
    QListWidgetItem* item = ui.listPending->currentItem();
    if (!item) return;

    int txnId = item->data(Qt::UserRole).toInt();

    QSqlQuery q = DatabaseManager::instance().executeQuery(
        "UPDATE onramp_transactions SET status = 'failed' WHERE id = ?", { txnId });

    if (q.numRowsAffected() > 0) {
        QMessageBox::information(this, "Success", "Transaction Rejected");
        loadPendingOnramp();
    }
}

void AdminPage::onRefreshClicked()
{
    loadPendingOnramp();
}