#include "P2PPage.h"
#include "services/ApiClient.h"
#include "core/AppState.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QNetworkReply>

P2PPage::P2PPage(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

    // Connect the send button to our slot
    connect(ui.btnSend, &QPushButton::clicked, this, &P2PPage::onSendClicked);
    
    // Optionally setup the spinbox properties if not done in designer
    ui.spinAmount->setMinimum(0.01);
    ui.spinAmount->setMaximum(100000.00); 
}

P2PPage::~P2PPage()
{}

void P2PPage::onSendClicked()
{
    QString recipient = ui.leRecipient->text().trimmed();
    double amount = ui.spinAmount->value();

    if (recipient.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter a recipient.");
        return;
    }

    if (amount <= 0) {
        QMessageBox::warning(this, "Validation Error", "Amount must be greater than zero.");
        return;
    }

    // 1. Disable button to prevent double submissions
    ui.btnSend->setEnabled(false);
    ui.btnSend->setText("Sending...");

    // 2. Prepare JSON payload
    QJsonObject body;
    body["recipient"] = recipient;
    body["amount"] = amount;

    // 3. Send API Request
    QNetworkReply* reply = ApiClient::instance().post("/transfer/p2p", body);

    // 4. Handle response asynchronously
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater(); // Prevent memory leak

        // Re-enable button
        ui.btnSend->setEnabled(true);
        ui.btnSend->setText("Send Securely");

        if (reply->error() == QNetworkReply::NoError) {
            // Success! Parse the updated balance if the API returns it
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject resObj = doc.object();

            // Notify user
            QMessageBox::information(this, "Success", "Transfer completed securely!");

            // Update global state (if the backend returns the new balance)
            if (resObj.contains("newBalance")) {
                AppState::instance().setBalance(resObj["newBalance"].toDouble());
            }

            // Clear the form
            ui.leRecipient->clear();
            ui.spinAmount->setValue(0.0);
            
            // Optionally: emit a signal here to tell the AppController to route back to Home/Dashboard
        } else {
            // Error! 
            QString errorMsg = reply->errorString();
            
            // Try to extract a cleaner message from the backend JSON if possible
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if(!doc.isEmpty() && doc.object().contains("error")) {
                errorMsg = doc.object()["error"].toString();
            }

            QMessageBox::critical(this, "Transfer Failed", "Error: " + errorMsg);
        }
    });
}

