#include "DashboardPage.h"
#include "DatabaseManager.h"
#include "core/AppState.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#include <QSqlQuery>
#include <QDateTime>
#include <QRegularExpression>
#include "components/PinDialog.h"
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "components/TransactionSuccessDialog.h"
#include "RechargeManager.h"
#include <QStackedWidget>
#include <QGridLayout>
#include <QRandomGenerator>

DashboardPage::DashboardPage(QWidget* parent) : QWidget(parent)
{
    ui.setupUi(this);

    // === ADD THIS LINE ===
    m_loadingOverlay = new LoadingOverlay(this);

    const AppState& state = AppState::instance();
    updateBalance(state.balance(), state.fullName());

    // Show loading before loading data
    m_loadingOverlay->showLoading("CalxSecure...");
    
    loadRealDataFromDB();
    rechargeManager = new RechargeManager(this);
    // Hide loading after data is loaded
    QTimer::singleShot(800, this, [this]() {
        m_loadingOverlay->hideLoading();
        });
    setupSidebarNavigation();
    // === TRANSFER FUNDS SETUP ===
    setupTransferPage();
    connect(ui.btnAddFundsConfirm, &QPushButton::clicked, this, &DashboardPage::onAddFundsConfirmClicked);
    // Button Connections
    connect(ui.btnChangePin, &QPushButton::clicked,
        this, &DashboardPage::onChangeTransactionPinClicked);

    connect(ui.btnResetPin, &QPushButton::clicked,
        this, &DashboardPage::onResetTransactionPinClicked);
    connect(ui.btnCreateBill, &QPushButton::clicked, this, &DashboardPage::onCreateNewBillClicked);
    connect(ui.btnLogout, &QPushButton::clicked, this, &DashboardPage::onLogoutClicked);
    connect(ui.btnShowBalance, &QPushButton::clicked, this, &DashboardPage::onShowBalanceClicked);
    connect(ui.btnPay, &QPushButton::clicked, this, &DashboardPage::onPayNowClicked);
    connect(ui.btnSendMoney, &QPushButton::clicked, this, &DashboardPage::onSendMoneyClicked);
    connect(ui.billsList, &QListWidget::itemClicked,
        this, &DashboardPage::onBillItemClicked);
    // P2P Real-time Validation
    connect(ui.mobileNumber, &QLineEdit::textChanged, this, &DashboardPage::validateP2PInput);
    connect(ui.p2pAmount, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &DashboardPage::validateP2PInput);


    connect(ui.btnAddFunds, &QPushButton::clicked, this, [this]() {
        ui.stackedWidget->setCurrentWidget(ui.Transfer);
        });
    // Recharge specific
    setupRechargePage();
    setupSettingsPage();
    setupRewardsPage();
    // Auto refresh balance
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &DashboardPage::refreshBalanceFromDB);
    timer->start(15000);
}

DashboardPage::~DashboardPage() {}
void DashboardPage::onBillItemClicked(QListWidgetItem* item)
{
    int billId = item->data(Qt::UserRole).toInt();

    const AppState& state = AppState::instance();
    int userId = state.userId().toInt();

    // Get bill details
    QSqlQuery q = DatabaseManager::instance().executeQuery(
        "SELECT amount, status, description FROM bills WHERE id = ?", { billId });

    if (!q.next()) return;

    double amount = q.value("amount").toDouble();
    QString status = q.value("status").toString();
    QString desc = q.value("description").toString();

    if (status == "paid") {
        showErrorMessage("Already Paid", "This bill is already paid.");
        return;
    }

    if (amount > currentBalance) {
        showErrorMessage("Insufficient Balance", "Not enough balance.");
        return;
    }

    if (!verifyTransactionPin()) return;

    if (m_loadingOverlay) {
        m_loadingOverlay->showLoading("Processing Bill...");
    }

    QTimer::singleShot(800, this, [=]() {

        DatabaseManager::instance().transactionBegin();

        bool ok1 = DatabaseManager::instance().executeQuery(
            "UPDATE users SET balance = balance - ? WHERE id = ?",
            { amount, userId }).numRowsAffected() > 0;

        bool ok2 = DatabaseManager::instance().executeQuery(
            "UPDATE bills SET status = 'paid' WHERE id = ?",
            { billId }).numRowsAffected() > 0;

        bool ok3 = DatabaseManager::instance().executeQuery(R"(
            INSERT INTO transactions (user_id, type, amount, recipient, description)
            VALUES (?, 'bill_payment', ?, ?, 'Bill Payment')
        )", { userId, amount, desc }).isActive();

        if (ok1 && ok2 && ok3) {
            DatabaseManager::instance().transactionCommit();

            currentBalance -= amount;
            // Fetch the full name directly from the Singleton instance
            updateBalance(currentBalance, AppState::instance().fullName());

            loadRealDataFromDB();

            if (m_loadingOverlay) m_loadingOverlay->hideLoading();

            TransactionSuccessDialog* successDlg = new TransactionSuccessDialog(this);
            successDlg->showSuccess(TransactionSuccessDialog::Payment,
                QString("₹%1").arg(amount, 0, 'f', 2),
                desc);
        }
        else {
            DatabaseManager::instance().transactionRollback();

            if (m_loadingOverlay) m_loadingOverlay->hideLoading();

            showErrorMessage("Failed", "Bill payment failed.");
        }
        });
}
void DashboardPage::setupTransferPage()
{
    // Clear any previous layout
    if (QLayout* oldLayout = ui.Transfer->layout()) {
        QLayoutItem* item;
        while ((item = oldLayout->takeAt(0)) != nullptr) delete item;
        delete oldLayout;
    }

    QVBoxLayout* mainLayout = new QVBoxLayout(ui.Transfer);
    mainLayout->setContentsMargins(32, 32, 32, 32);
    mainLayout->setSpacing(24);

    // Title
    QLabel* title = new QLabel("Transfer Funds");
    title->setStyleSheet("font-size: 32px; font-weight: 700; color: #ffffff;");
    mainLayout->addWidget(title);

    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(28);

    // ================= LEFT: INPUT CARD =================
    QFrame* inputCard = new QFrame();
    inputCard->setObjectName("transferInputCard");
    inputCard->setStyleSheet(R"(
        QFrame#transferInputCard {
            background-color: #111113;
            border: 1px solid #27272a;
            border-radius: 20px;
            padding: 32px;
        }
    )");
    QVBoxLayout* inputL = new QVBoxLayout(inputCard);
    inputL->setSpacing(20);

    inputL->addWidget(new QLabel("<b style='font-size:22px'>Add Money</b>"));

    inputL->addWidget(ui.amountInput);

    inputL->addWidget(new QLabel("Payment Method"));
    ui.paymentMethod->clear();
    ui.paymentMethod->addItems({ "UPI / Bank Transfer", "Debit / Credit Card", "Net Banking" });
    inputL->addWidget(ui.paymentMethod);

    inputL->addWidget(ui.cardDetailsWidget);

    inputL->addStretch();
    inputL->addWidget(ui.btnAddFundsConfirm);

    contentLayout->addWidget(inputCard, 1);

    // ================= RIGHT: HISTORY =================
    QFrame* histCard = new QFrame();
    histCard->setObjectName("historyCard");
    histCard->setStyleSheet(R"(
        QFrame#historyCard {
            background-color: #111113;
            border: 1px solid #27272a;
            border-radius: 20px;
            padding: 24px;
        }
    )");
    QVBoxLayout* histL = new QVBoxLayout(histCard);
    histL->addWidget(new QLabel("<b style='font-size:20px'>On-Ramp History</b>"));

    ui.onrampHistoryList->clear();
    histL->addWidget(ui.onrampHistoryList);

    contentLayout->addWidget(histCard, 1);

    mainLayout->addLayout(contentLayout);

    // Connect payment method change for dynamic fields
    connect(ui.paymentMethod, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &DashboardPage::onPaymentMethodChanged);

    loadOnrampHistory();
}

// New method
void DashboardPage::onPaymentMethodChanged(int index)
{
    ui.cardDetailsWidget->setVisible(index == 1); // Only show for Card
}


// ====================== BALANCE ======================
void DashboardPage::updateBalance(double balance, const QString& fullName)
{
    currentBalance = balance;
    ui.greeting->setText("Hi, " + fullName);

    if (balanceVisible) {
        ui.balanceAmount->setText(QString("₹ %1").arg(balance, 0, 'f', 2));
    }
    else {
        ui.balanceAmount->setText("₹ ••••");
    }
}
// ====================== LOAD REAL DATA ======================
void DashboardPage::loadRealDataFromDB()
{
    const AppState& state = AppState::instance();
    if (state.userId().isEmpty()) return;

    bool ok;
    int userId = state.userId().toInt(&ok);
    if (!ok) return;

    // ================= BALANCE =================
    QSqlQuery bal = DatabaseManager::instance().executeQuery(
        "SELECT balance FROM users WHERE id = ?", { userId });

    if (bal.next()) {
        currentBalance = bal.value(0).toDouble();
        updateBalance(currentBalance, state.fullName());
    }

    // ================= TRANSACTIONS =================
    ui.transactionsList->clear();

    QSqlQuery tx = DatabaseManager::instance().executeQuery(R"(
        SELECT type, amount, recipient, created_at
        FROM transactions
        WHERE from_user_id = ?
        ORDER BY created_at DESC LIMIT 8
    )", { userId });

    while (tx.next()) {
        QString type = tx.value("type").toString();
        double amt = tx.value("amount").toDouble();
        QString rec = tx.value("recipient").toString();
        QString time = tx.value("created_at").toDateTime().toString("dd MMM hh:mm");

        QString sign = type.contains("send") ? "−" : "+";

        QString text = QString("%1   %2 %3   %4₹%5")
            .arg(time)
            .arg(type.contains("send") ? "To" : "From")
            .arg(rec.left(12))
            .arg(sign)
            .arg(amt, 0, 'f', 2);

        QListWidgetItem* item = new QListWidgetItem(text);
        item->setForeground(type.contains("send") ? QColor("#ef4444") : QColor("#22c55e"));

        ui.transactionsList->addItem(item);
    }
    qDebug() << "Current User ID:" << userId;
    // ================= BILLS =================
    ui.billsList->clear();
    ui.billsList->setSpacing(12); // Add spacing between the cards

    QSqlQuery bill = DatabaseManager::instance().executeQuery(R"(
        SELECT id, description, amount, due_date, status
        FROM bills
        WHERE user_id = ?
        ORDER BY due_date
        LIMIT 5
    )", { userId });

    while (bill.next()) {
        int billId = bill.value("id").toInt();
        QString desc = bill.value("description").toString();
        double amt = bill.value("amount").toDouble();
        QString status = bill.value("status").toString();
        QString due = bill.value("due_date").toDateTime().toString("dd MMM yyyy");

        // 1. Create Custom Widget Card
        QFrame* card = new QFrame();
        card->setStyleSheet(R"(
            QFrame {
                background-color: #18181b;
                border-radius: 8px;
                border: 1px solid #27272a;
            }
            QLabel { 
                border: none; 
                background: transparent; 
            }
        )");

        QVBoxLayout* layout = new QVBoxLayout(card);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(8);

        // 2. Top Layout: Description & Amount
        QHBoxLayout* topLayout = new QHBoxLayout();
        QLabel* lblDesc = new QLabel(desc);
        lblDesc->setStyleSheet("color: #ffffff; font-size: 16px; font-weight: bold;");

        QLabel* lblAmt = new QLabel(QString("₹%1").arg(amt, 0, 'f', 2));
        lblAmt->setStyleSheet("color: #ffffff; font-size: 16px; font-weight: bold;");
        lblAmt->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        topLayout->addWidget(lblDesc);
        topLayout->addWidget(lblAmt);

        // 3. Bottom Layout: Due Date & Status
        QHBoxLayout* bottomLayout = new QHBoxLayout();
        QLabel* lblDue = new QLabel("Due: " + due);
        lblDue->setStyleSheet("color: #a1a1aa; font-size: 13px;");

        QLabel* lblStatus = new QLabel(status.toUpper());
        lblStatus->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        if (status == "paid") {
            lblStatus->setStyleSheet("color: #22c55e; font-size: 13px; font-weight: bold;");
        }
        else {
            lblStatus->setStyleSheet("color: #facc15; font-size: 13px; font-weight: bold;");
        }

        bottomLayout->addWidget(lblDue);
        bottomLayout->addWidget(lblStatus);

        layout->addLayout(topLayout);
        layout->addLayout(bottomLayout);

        // 4. Create and add ListWidgetItem with Custom Widget
        QListWidgetItem* item = new QListWidgetItem(ui.billsList);
        item->setData(Qt::UserRole, billId);
        item->setSizeHint(QSize(0, 90));
        ui.billsList->setItemWidget(item, card);
    }

    // ================= REWARDS (Dynamic) =================
    ui.rewardsList->clear();
    QSqlQuery rew = DatabaseManager::instance().executeQuery(R"(
        SELECT r.id, r.title, r.amount, r.description, r.type
        FROM rewards r
        LEFT JOIN user_rewards ur ON r.id = ur.reward_id AND ur.user_id = ?
        WHERE ur.id IS NULL
        ORDER BY r.amount DESC
    )", { userId });

    int loaded = 0;
    while (rew.next()) {
        QString title = rew.value("title").toString();
        double amt = rew.value("amount").toDouble();
        QString desc = rew.value("description").toString();

        QString text = QString("%1\n₹%2\n%3").arg(title).arg(amt, 0, 'f', 2).arg(desc);

        QListWidgetItem* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, rew.value("id").toInt());
        ui.rewardsList->addItem(item);
        loaded++;
    }

    if (loaded == 0) {
        ui.rewardsList->addItem("No unclaimed rewards available.\nAll rewards claimed!");
    }
    else {
        qDebug() << "✅ Loaded" << loaded << "rewards successfully";
    }
    if (ui.rewardsList->count() == 0) {
        ui.rewardsList->addItem("Daily Login Bonus\n₹2.50\nLog in every day");
        ui.rewardsList->addItem("Scratch & Win\n₹4.00\nLucky scratch card");
        ui.rewardsList->addItem("Welcome Bonus\n₹5.00\nNew user reward");
    }
}
void DashboardPage::setupRewardsPage()
{
    if (QLayout* old = ui.Rewards->layout()) {
        QLayoutItem* item;
        while ((item = old->takeAt(0))) delete item;
        delete old;
    }

    QVBoxLayout* mainLayout = new QVBoxLayout(ui.Rewards);
    mainLayout->setContentsMargins(32, 32, 32, 32);
    mainLayout->setSpacing(24);

    QLabel* title = new QLabel("Rewards & Cashback");
    title->setStyleSheet("font-size: 32px; font-weight: 700; color: #ffffff;");
    mainLayout->addWidget(title);

    // Total Rewards Card (simple)
    QFrame* totalCard = new QFrame();
    totalCard->setStyleSheet("background-color: #18181b; border: 1px solid #27272a; border-radius: 20px; padding: 24px;");
    QVBoxLayout* totalL = new QVBoxLayout(totalCard);
    totalL->addWidget(new QLabel("Total Rewards Earned"));
    QLabel* totalAmount = new QLabel("₹0.00");
    totalAmount->setStyleSheet("font-size: 48px; font-weight: 700; color: #22c55e;");
    totalL->addWidget(totalAmount);
    mainLayout->addWidget(totalCard);

    // Rewards List
    ui.rewardsList->setStyleSheet(R"(
        QListWidget::item {
            padding: 20px;
            margin: 8px;
            border-radius: 16px;
            background-color: #1f1f23;
        }
        QListWidget::item:hover {
            background-color: #27272a;
        }
    )");
    mainLayout->addWidget(ui.rewardsList);

    // Scratch Button
    QPushButton* scratchBtn = new QPushButton("🎟️ Scratch Card (₹0.5 - ₹5.0)");
    scratchBtn->setStyleSheet("background-color: #eab308; color: black; padding: 18px; font-size: 18px; font-weight: bold; border-radius: 16px;");
    connect(scratchBtn, &QPushButton::clicked, this, &DashboardPage::onScratchCardClicked);
    mainLayout->addWidget(scratchBtn);
    if (ui.rewardsList->count() == 0) {
        // Dummy 1
        QListWidgetItem* dummy1 = new QListWidgetItem("Daily Login Bonus\n₹2.50\nLog in every day");
        dummy1->setBackground(QColor("#1f1f23"));
        ui.rewardsList->addItem(dummy1);

        // Dummy 2
        QListWidgetItem* dummy2 = new QListWidgetItem("Scratch & Win\n₹4.00\nLucky scratch card");
        dummy2->setBackground(QColor("#eab308"));
        dummy2->setForeground(QColor("black"));
        ui.rewardsList->addItem(dummy2);

        // Dummy 3
        QListWidgetItem* dummy3 = new QListWidgetItem("Recharge Cashback\n₹3.00\nOn any recharge");
        dummy3->setBackground(QColor("#1f1f23"));
        ui.rewardsList->addItem(dummy3);

        qDebug() << "Added 3 dummy reward cards";
    }
}
void DashboardPage::loadAvailableRewards(QListWidget* list)
{
    list->clear();

    const AppState& state = AppState::instance();
    int userId = state.userId().toInt();

    QSqlQuery q = DatabaseManager::instance().executeQuery(R"(
        SELECT r.id, r.title, r.amount, r.description, r.type
        FROM rewards r
        LEFT JOIN user_rewards ur ON r.id = ur.reward_id AND ur.user_id = ?
        WHERE ur.id IS NULL
        ORDER BY r.amount DESC
    )", { userId });

    while (q.next()) {
        QString title = q.value("title").toString();
        double amount = q.value("amount").toDouble();
        QString desc = q.value("description").toString();
        QString type = q.value("type").toString();

        QString text = QString("%1\n₹%2\n%3").arg(title).arg(amount, 0, 'f', 2).arg(desc);

        QListWidgetItem* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, q.value("id").toInt());

        if (type == "scratch") {
            item->setBackground(QColor("#eab308"));
            item->setForeground(QColor("black"));
        }
        else {
            item->setBackground(QColor("#1f1f23"));
        }

        list->addItem(item);
    }

    if (list->count() == 0) {
        list->addItem("No rewards available right now");
    }
}
void DashboardPage::onScratchCardClicked()
{
    if (!verifyTransactionPin()) return;

    if (m_loadingOverlay) m_loadingOverlay->showLoading("Scratching...");

    QTimer::singleShot(1200, this, [this]() {
        if (m_loadingOverlay) m_loadingOverlay->hideLoading();

        // Generate random reward between ₹0.50 - ₹5.00
        double rewardAmount = 0.5 + (QRandomGenerator::global()->bounded(450) / 100.0);

        // Add to balance
        const AppState& state = AppState::instance();
        int userId = state.userId().toInt();

        DatabaseManager::instance().executeQuery(
            "UPDATE users SET balance = balance + ? WHERE id = ?",
            { rewardAmount, userId });

        currentBalance += rewardAmount;
        updateBalance(currentBalance, state.fullName());
        loadRealDataFromDB();

        // Show success with confetti
        TransactionSuccessDialog* success = new TransactionSuccessDialog(this);
        success->showSuccess(TransactionSuccessDialog::Credit,
            QString("₹%1").arg(rewardAmount, 0, 'f', 2),
            "Scratch Card Reward Claimed! 🎉");

        showSuccessMessage("Reward Claimed", QString("You won ₹%1 from scratch card!").arg(rewardAmount, 0, 'f', 2));
        });
}
// ====================== REFRESH BALANCE ======================
void DashboardPage::refreshBalanceFromDB()
{
    const AppState& state = AppState::instance();
    if (state.userId().isEmpty()) return;

    bool ok;
    int uid = state.userId().toInt(&ok);
    if (!ok) return;

    QSqlQuery q = DatabaseManager::instance().executeQuery(
        "SELECT balance FROM users WHERE id = ?", { uid });

    if (q.next()) {
        double newBal = q.value(0).toDouble();
        if (!qFuzzyCompare(currentBalance, newBal)) {
            currentBalance = newBal;
            updateBalance(currentBalance, state.fullName());
        }
    }
}

// ====================== SECURE PIN VERIFICATION ======================
bool DashboardPage::verifyTransactionPin()
{
    if (m_loadingOverlay) m_loadingOverlay->showLoading("Verifying Security...");

    const AppState& state = AppState::instance();
    int userId = state.userId().toInt();

    QSqlQuery q = DatabaseManager::instance().executeQuery(
        "SELECT transaction_pin FROM users WHERE id = ?", { userId });

    QString dbPin = q.next() ? q.value(0).toString() : "";

    if (m_loadingOverlay) m_loadingOverlay->hideLoading();

    if (dbPin.isEmpty()) {
        // Setup new PIN
        PinDialog* setupDialog = new PinDialog(true, this);
        bool created = false;

        connect(setupDialog, &PinDialog::pinCreated, this, [&](const QString& pin) {
            DatabaseManager::instance().executeQuery(
                "UPDATE users SET transaction_pin = ? WHERE id = ?", { pin, userId });
            showSuccessMessage("Success", "Transaction PIN created successfully!");
            created = true;
            });

        setupDialog->setAttribute(Qt::WA_DeleteOnClose);
        setupDialog->show();           // Use show() since it's QWidget
        // Wait for dialog to close using event loop
        QEventLoop loop;
        connect(setupDialog, &PinDialog::destroyed, &loop, &QEventLoop::quit);
        loop.exec();

        return created;
    }

    // Normal PIN verification
    PinDialog* dialog = new PinDialog(false, this);
    dialog->setAttribute(Qt::WA_TranslucentBackground);
    dialog->setStyleSheet(R"(
        QWidget {
            background-color: rgba(10, 10, 15, 235);
            border: 1px solid #3b82f6;
            border-radius: 16px;
        }
    )");

    bool verified = false;

    connect(dialog, &PinDialog::pinEntered, this, [&](const QString& pin) {
        if (pin == dbPin) {
            verified = true;
        }
        else {
            QLabel* err = dialog->findChild<QLabel*>("lblError");
            if (err) {
                err->setText("Incorrect PIN");
                err->setVisible(true);
            }
        }
        dialog->close();   // Safe for QWidget
        });

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();

    // Wait for dialog to close
    QEventLoop loop;
    connect(dialog, &PinDialog::destroyed, &loop, &QEventLoop::quit);
    loop.exec();

    return verified;
}
// ====================== CHANGE PIN ======================
void DashboardPage::onChangeTransactionPinClicked()
{
    if (!verifyTransactionPin()) return;

    bool ok1, ok2;
    QString newPin = QInputDialog::getText(this, "Change PIN",
        "Enter new 4-digit PIN:", QLineEdit::Password, "", &ok1);

    if (!ok1 || newPin.length() != 4) {
        showErrorMessage("Invalid PIN", "PIN must be 4 digits.");
        return;
    }

    QString confirmPin = QInputDialog::getText(this, "Confirm PIN",
        "Confirm new 4-digit PIN:", QLineEdit::Password, "", &ok2);

    if (!ok2 || newPin != confirmPin) {
        showErrorMessage("PIN Mismatch", "PINs do not match.");
        return;
    }

    const AppState& state = AppState::instance();
    int userId = state.userId().toInt();

    if (DatabaseManager::instance().executeQuery(
        "UPDATE users SET transaction_pin = ? WHERE id = ?", { newPin, userId }).numRowsAffected() > 0) {
        showSuccessMessage("Success", "Transaction PIN changed successfully!");
    }
    else {
        showErrorMessage("Error", "Failed to update PIN.");
    }
}

// ====================== RESET PIN ======================
void DashboardPage::onResetTransactionPinClicked()
{
    const AppState& state = AppState::instance();
    if (state.userId().isEmpty()) {
        showErrorMessage("Error", "Please login again.");
        return;
    }

    if (!verifyCurrentPinForReset()) return;

    bool ok1, ok2;
    QString newPin = QInputDialog::getText(this, "Reset Transaction PIN",
        "Enter NEW 4-digit Transaction PIN:", QLineEdit::Password, "", &ok1);

    if (!ok1 || newPin.length() != 4 || !QRegularExpression("^\\d{4}$").match(newPin).hasMatch()) {
        showErrorMessage("Invalid PIN", "PIN must be exactly 4 digits (0-9).");
        return;
    }

    QString confirmPin = QInputDialog::getText(this, "Confirm New PIN",
        "Confirm your new 4-digit PIN:", QLineEdit::Password, "", &ok2);

    if (!ok2 || newPin != confirmPin) {
        showErrorMessage("PIN Mismatch", "The two PINs do not match.");
        return;
    }

    int userId = state.userId().toInt();

    bool updated = DatabaseManager::instance().executeQuery(
        "UPDATE users SET transaction_pin = ? WHERE id = ?", { newPin, userId }).numRowsAffected() > 0;

    if (updated) {
        showSuccessMessage("PIN Reset Successful",
            "Your Transaction PIN has been successfully reset.\n"
            "Use the new PIN for all future transactions.");
    }
    else {
        showErrorMessage("Failed", "Could not reset PIN. Please try again.");
    }
}

bool DashboardPage::verifyCurrentPinForReset()
{
    return verifyTransactionPin(); // We can re-use the exact same PinDialog verification logic!
}

void DashboardPage::setupRechargePage()
{
    // Clear old layout if any
    if (QLayout* old = ui.Recharge->layout()) {
        QLayoutItem* item;
        while ((item = old->takeAt(0))) delete item;
        delete old;
    }

    QVBoxLayout* mainL = new QVBoxLayout(ui.Recharge);
    mainL->setContentsMargins(32, 32, 32, 32);
    mainL->setSpacing(20);

    QLabel* title = new QLabel("Mobile Recharge");
    title->setStyleSheet("font-size: 32px; font-weight: 700;");
    mainL->addWidget(title);

    // Number Input
    rechargeNumberEdit = new QLineEdit();
    rechargeNumberEdit->setPlaceholderText("Enter 10-digit mobile number");
    rechargeNumberEdit->setMaxLength(10);
    mainL->addWidget(rechargeNumberEdit);

    // Operator
    rechargeOperatorCombo = new QComboBox();
    for (const auto& op : rechargeManager->getOperators()) {
        rechargeOperatorCombo->addItem(op.name, op.code);
    }
    mainL->addWidget(rechargeOperatorCombo);

    QPushButton* fetchBtn = new QPushButton("Show Plans");
    fetchBtn->setStyleSheet("background: #22c55e; color: black; padding: 14px; font-weight: bold;");
    connect(fetchBtn, &QPushButton::clicked, this, &DashboardPage::onFetchRechargePlans);
    mainL->addWidget(fetchBtn);

    // Plans List
    plansListWidget = new QListWidget();
    plansListWidget->setStyleSheet("QListWidget::item { padding: 16px; margin: 4px; border-radius: 12px; }");
    connect(plansListWidget, &QListWidget::itemClicked, this, &DashboardPage::onRechargePlanSelected);
    mainL->addWidget(plansListWidget);

    QPushButton* payBtn = new QPushButton("Proceed to Pay");
    payBtn->setStyleSheet("background: #22c55e; color: black; padding: 16px; font-size: 17px; font-weight: bold;");
    connect(payBtn, &QPushButton::clicked, this, &DashboardPage::onRechargeProceedToPay);
    mainL->addWidget(payBtn);
}

void DashboardPage::onFetchRechargePlans()
{
    QString number = rechargeNumberEdit->text().trimmed();
    if (number.length() != 10) {
        showErrorMessage("Invalid Number", "Please enter 10-digit number");
        return;
    }

    QString opCode = rechargeOperatorCombo->currentData().toString();
    auto plans = rechargeManager->getPlans(opCode);

    plansListWidget->clear();
    for (const auto& plan : plans) {
        QString text = QString("%1\n₹%2 - %3").arg(plan.name).arg(plan.amount).arg(plan.description);
        QListWidgetItem* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, plan.id);
        plansListWidget->addItem(item);
    }
}

void DashboardPage::onRechargePlanSelected(QListWidgetItem* item)
{
    // In real app you would store full plan, here we use mock
    selectedRechargePlan.amount = 239; // example
    selectedRechargePlan.name = item->text().split('\n').first();
}

void DashboardPage::onRechargeProceedToPay()
{
    if (selectedRechargePlan.amount <= 0) {
        showErrorMessage("No Plan", "Please select a plan");
        return;
    }

    QString number = rechargeNumberEdit->text().trimmed();
    if (number.length() != 10) return;

    if (!verifyTransactionPin()) return;

    if (m_loadingOverlay) m_loadingOverlay->showLoading("Processing Recharge...");

    QTimer::singleShot(1200, this, [this, number]() {
        // Simulate success
        if (m_loadingOverlay) m_loadingOverlay->hideLoading();

        TransactionSuccessDialog* success = new TransactionSuccessDialog(this);
        success->showSuccess(TransactionSuccessDialog::Recharge,
            QString("₹%1").arg(selectedRechargePlan.amount),
            QString("Recharged %1 successfully").arg(number));

        // Clear fields
        rechargeNumberEdit->clear();
        plansListWidget->clear();
        });
}

// ====================== P2P ======================
void DashboardPage::validateP2PInput()
{
    evaluateSendButtonState();
}

void DashboardPage::evaluateSendButtonState()
{
    QString mobile = ui.mobileNumber->text().trimmed();
    double amount = ui.p2pAmount->value();
    
    // Mobile validation
    bool mobileValid = false;
    if (mobile.isEmpty()) {
        ui.lblMobileError->setVisible(false);
    } else if (mobile.length() != 10 || !QRegularExpression("^\\d{10}$").match(mobile).hasMatch()) {
        ui.lblMobileError->setText("Enter valid 10-digit number");
        ui.lblMobileError->setVisible(true);
    } else {
        ui.lblMobileError->setVisible(false);
        mobileValid = true;
    }
    
    // Amount validation
    bool amountValid = false;
    if (amount <= 0) {
        if (amount > 0 || ui.p2pAmount->hasFocus()) {
            ui.lblAmountError->setText("Enter valid amount");
            ui.lblAmountError->setVisible(true);
        } else {
            ui.lblAmountError->setVisible(false);
        }
    } else if (amount > currentBalance) {
        ui.lblAmountError->setText("Not enough balance");
        ui.lblAmountError->setVisible(true);
    } else {
        ui.lblAmountError->setVisible(false);
        amountValid = true;
    }
    
    // Enable button only if both valid
    ui.btnPay->setEnabled(mobileValid && amountValid);
}

void DashboardPage::onSendMoneyClicked()
{
    ui.mobileNumber->clear();
    ui.p2pAmount->setValue(0.0);
    ui.lblMobileError->setVisible(false);
    ui.lblAmountError->setVisible(false);
    ui.btnPay->setEnabled(false);
    ui.stackedWidget->setCurrentWidget(ui.P2PTransfer);
    ui.mobileNumber->setFocus();
}

void DashboardPage::onPayNowClicked()
{
    QString mobile = ui.mobileNumber->text().trimmed();
    double amount = ui.p2pAmount->value();

    ui.lblMobileError->setVisible(false);
    ui.lblAmountError->setVisible(false);

    if (mobile.length() != 10 || !QRegularExpression("^\\d{10}$").match(mobile).hasMatch()) {
        ui.lblMobileError->setText("Enter valid 10-digit number");
        ui.lblMobileError->setVisible(true);
        return;
    }

    if (amount <= 0 || amount > currentBalance) {
        ui.lblAmountError->setText(amount > currentBalance ? "Insufficient balance" : "Enter valid amount");
        ui.lblAmountError->setVisible(true);
        return;
    }

    // Check if recipient exists in DB
    QSqlQuery check = DatabaseManager::instance().executeQuery(
        "SELECT id, full_name FROM users WHERE phone = ?", { mobile });

    if (!check.next()) {
        ui.lblMobileError->setText("User not found with this number");
        ui.lblMobileError->setVisible(true);
        return;
    }

    QString recipientName = check.value("full_name").toString();

    if (!verifyTransactionPin()) return;

    if (m_loadingOverlay) m_loadingOverlay->showLoading("Transferring...");

    QTimer::singleShot(900, this, [this, amount, mobile, recipientName]() {
        const AppState& state = AppState::instance();
        int userId = state.userId().toInt();

        DatabaseManager::instance().transactionBegin();

        bool ok1 = DatabaseManager::instance().executeQuery(
            "UPDATE users SET balance = balance - ? WHERE id = ?", { amount, userId }).numRowsAffected() > 0;

        bool ok2 = DatabaseManager::instance().executeQuery(
            "UPDATE users SET balance = balance + ? WHERE phone = ?", { amount, mobile }).numRowsAffected() > 0;

        if (ok1 && ok2) {
            DatabaseManager::instance().executeQuery(R"(
                INSERT INTO transactions (from_user_id, type, amount, recipient, description, status)
                VALUES (?, 'p2p_send', ?, ?, ?, 'completed')
            )", { userId, amount, mobile, "P2P to " + recipientName });

            DatabaseManager::instance().transactionCommit();

            currentBalance -= amount;
            updateBalance(currentBalance, state.fullName());
            loadRealDataFromDB();

            if (m_loadingOverlay) m_loadingOverlay->hideLoading();

            // Use Fancy Success Dialog
            TransactionSuccessDialog* success = new TransactionSuccessDialog(this);
            success->showSuccess(TransactionSuccessDialog::Debit,
                QString("₹%1").arg(amount, 0, 'f', 2),
                QString("Sent to %1 (%2)").arg(recipientName).arg(mobile));
        }
        else {
            DatabaseManager::instance().transactionRollback();
            if (m_loadingOverlay) m_loadingOverlay->hideLoading();
            showErrorMessage("Transfer Failed", "Transaction could not be completed.");
        }

        // Reset fields
        ui.mobileNumber->clear();
        ui.p2pAmount->setValue(0);
        ui.btnPay->setEnabled(false);
        });
}
void DashboardPage::onAddFundsConfirmClicked()
{
    double amount = ui.amountInput->value();
    if (amount <= 0) {
        ui.lblAmountError->setText("Amount must be greater than zero");
        ui.lblAmountError->setVisible(true);
        return;
    }
    ui.lblAmountError->setVisible(false);

    // Verify PIN only once
    if (!verifyTransactionPin()) return;

    if (m_loadingOverlay) m_loadingOverlay->showLoading("Adding Funds...");

    QTimer::singleShot(1000, this, [this, amount]() {
        const AppState& state = AppState::instance();
        int userId = state.userId().toInt();

        DatabaseManager::instance().transactionBegin();

        bool ok = DatabaseManager::instance().executeQuery(
            "UPDATE users SET balance = balance + ? WHERE id = ?",
            { amount, userId }).numRowsAffected() > 0;

        if (ok) {
            DatabaseManager::instance().executeQuery(R"(
                INSERT INTO onramp_transactions (user_id, amount, payment_method, provider, status)
                VALUES (?, ?, ?, ?, 'success')
            )", { userId, amount, ui.paymentMethod->currentText(), "Bank" });

            DatabaseManager::instance().transactionCommit();

            currentBalance += amount;
            updateBalance(currentBalance, state.fullName());
            loadRealDataFromDB();
            loadOnrampHistory();

            TransactionSuccessDialog* dlg = new TransactionSuccessDialog(this);
            dlg->showSuccess(TransactionSuccessDialog::Credit,
                QString("₹%1").arg(amount, 0, 'f', 2),
                "Funds added to wallet");
        }
        else {
            DatabaseManager::instance().transactionRollback();
            showErrorMessage("Failed", "Could not add funds.");
        }

        if (m_loadingOverlay) m_loadingOverlay->hideLoading();
        });
}
void DashboardPage::setupSettingsPage()
{
    if (QLayout* old = ui.Settings->layout()) {
        QLayoutItem* item;
        while ((item = old->takeAt(0)) != nullptr) delete item;
        delete old;
    }

    QVBoxLayout* mainLayout = new QVBoxLayout(ui.Settings);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(24);

    // Title
    QLabel* title = new QLabel("Settings");
    title->setStyleSheet("font-size: 36px; font-weight: 700; color: #ffffff;");
    mainLayout->addWidget(title);

    // Main Card
    QFrame* card = new QFrame();
    card->setObjectName("settingsCard");
    card->setStyleSheet(R"(
        QFrame#settingsCard {
            background-color: #111113;
            border: 1px solid #27272a;
            border-radius: 24px;
            padding: 32px;
        }
    )");
    QVBoxLayout* cardL = new QVBoxLayout(card);
    cardL->setSpacing(20);

    const AppState& state = AppState::instance();

    // Profile (Dynamic)
    cardL->addWidget(new QLabel("<b>Profile Information</b>"));
    cardL->addWidget(new QLabel("Full Name: " + state.fullName()));
    cardL->addWidget(new QLabel("Phone: " + state.phone()));
    cardL->addWidget(new QLabel("Email: " + state.email()));

    QPushButton* editBtn = new QPushButton("Edit Profile");
    editBtn->setStyleSheet("background: #3b82f6; color: white; padding: 12px;");
    cardL->addWidget(editBtn);

    cardL->addSpacing(16);

    // Security
    cardL->addWidget(new QLabel("<b>Security</b>"));
    cardL->addWidget(ui.btnChangePin);
    cardL->addWidget(ui.btnResetPin);

    cardL->addSpacing(16);

    // Appearance
    cardL->addWidget(new QLabel("<b>Appearance</b>"));
    QPushButton* themeBtn = new QPushButton("Toggle Dark / Light Theme");
    themeBtn->setStyleSheet("background: #64748b; color: white; padding: 12px;");
    cardL->addWidget(themeBtn);

    cardL->addSpacing(16);

    // Account
    cardL->addWidget(new QLabel("<b>Account</b>"));
    cardL->addWidget(ui.btnLogout);

    mainLayout->addWidget(card);
    mainLayout->addStretch();
}
// Improved History Loader
void DashboardPage::loadOnrampHistory()
{
    if (!ui.onrampHistoryList) return;
    ui.onrampHistoryList->clear();

    const AppState& state = AppState::instance();
    int userId = state.userId().toInt();

    QSqlQuery q = DatabaseManager::instance().executeQuery(R"(
        SELECT amount, payment_method, provider, status, created_at 
        FROM onramp_transactions 
        WHERE user_id = ? 
        ORDER BY created_at DESC
    )", { userId });

    while (q.next()) {
        double amt = q.value("amount").toDouble();
        QString method = q.value("payment_method").toString();
        QString provider = q.value("provider").toString();
        QString status = q.value("status").toString().toLower();
        QString time = q.value("created_at").toDateTime().toString("dd MMM hh:mm");

        QString display = QString("%1  •  ₹%2 via %3 (%4)")
            .arg(time)
            .arg(amt, 0, 'f', 2)
            .arg(provider.isEmpty() ? method : provider)
            .arg(status.toUpper());

        QListWidgetItem* item = new QListWidgetItem(display);

        if (status == "success")      item->setForeground(QColor("#22c55e"));
        else if (status == "failed")  item->setForeground(QColor("#ef4444"));
        else                          item->setForeground(QColor("#eab308"));

        ui.onrampHistoryList->addItem(item);
    }

    if (ui.onrampHistoryList->count() == 0) {
        ui.onrampHistoryList->addItem("No transactions yet");
    }
}
// ====================== MISC ======================
void DashboardPage::onShowBalanceClicked()
{
    // If it's already visible, just hide it immediately (no PIN needed)
    if (balanceVisible) {
        balanceVisible = false;
        ui.btnShowBalance->setText("Show Balance");
        updateBalance(currentBalance, AppState::instance().fullName());
        return;
    }

    // Attempting to show - require PIN
    if (!verifyTransactionPin()) return;

    balanceVisible = true;
    ui.btnShowBalance->setText("Hide Balance");
    updateBalance(currentBalance, AppState::instance().fullName());

    // Auto-hide after 15 seconds for security
    QTimer::singleShot(15000, this, [this]() {
        if (balanceVisible) {
            balanceVisible = false;
            ui.btnShowBalance->setText("Show Balance");
            updateBalance(currentBalance, AppState::instance().fullName());
        }
    });
}

void DashboardPage::onLogoutClicked()
{
    if (QMessageBox::question(this, "Logout", "Are you sure you want to logout?") == QMessageBox::Yes) {
        emit logoutRequested();
    }
}

void DashboardPage::setupSidebarNavigation()
{
    connect(ui.Home_pushButton, &QPushButton::clicked, this, [this]() {
        emit homeRequested();
        });
    QList<QPushButton*> buttons = {
         ui.Dashboard_pushButton, ui.Transfer_pushButton,
        ui.P2PTransfer_pushButton, ui.Bills_pushButton,
        ui.Recharge_pushButton, ui.Rewards_pushButton ,ui.Settings_pushButton
    };

    for (int i = 0; i < buttons.size(); ++i) {
        connect(buttons[i], &QPushButton::clicked, this, [this, i, buttons]() {
            ui.stackedWidget->setCurrentIndex(i);
            for (auto* btn : buttons) btn->setChecked(false);
            buttons[i]->setChecked(true);
            });
    }
}

void DashboardPage::setCurrentTab(int index)
{
    QList<QPushButton*> buttons = {
         ui.Dashboard_pushButton, ui.Transfer_pushButton,
        ui.P2PTransfer_pushButton, ui.Bills_pushButton,
        ui.Recharge_pushButton, ui.Rewards_pushButton ,ui.Settings_pushButton
    };

    if (index >= 0 && index < buttons.size()) {
        ui.stackedWidget->setCurrentIndex(index);
        for (auto* btn : buttons) btn->setChecked(false);
        buttons[index]->setChecked(true);
    }
}
void DashboardPage::loadBills(int userId)
{
    ui.billsList->clear();
    ui.billsList->setSpacing(14);

    QSqlQuery query = DatabaseManager::instance().executeQuery(R"(
        SELECT id, description, amount, due_date, status 
        FROM bills 
        WHERE user_id = ? 
        ORDER BY 
            CASE WHEN status = 'pending' THEN 1 ELSE 2 END,
            due_date ASC
    )", { userId });

    while (query.next()) {
        int billId = query.value("id").toInt();
        QString desc = query.value("description").toString();
        double amount = query.value("amount").toDouble();
        QDateTime dueDate = query.value("due_date").toDateTime();
        QString status = query.value("status").toString().toLower();

        QString dueStr = dueDate.toString("dd MMM yyyy");

        // Professional Bill Card
        QFrame* card = new QFrame();
        card->setMinimumHeight(100);
        card->setStyleSheet(R"(
            QFrame {
                background-color: #18181b;
                border: 1px solid #27272a;
                border-radius: 12px;
            }
            QFrame:hover {
                border: 1px solid #3b82f6;
                background-color: #1f1f23;
            }
        )");

        QVBoxLayout* mainLayout = new QVBoxLayout(card);
        mainLayout->setContentsMargins(16, 14, 16, 14);
        mainLayout->setSpacing(10);

        // Top Row: Description + Amount
        QHBoxLayout* topLayout = new QHBoxLayout();
        QLabel* descLabel = new QLabel(desc);
        descLabel->setStyleSheet("font-size: 15px; font-weight: 600; color: #f1f5f9;");
        descLabel->setWordWrap(true);

        QLabel* amountLabel = new QLabel("₹ " + QString::number(amount, 'f', 2));
        amountLabel->setStyleSheet("font-size: 17px; font-weight: 700; color: #ffffff;");
        amountLabel->setAlignment(Qt::AlignRight);

        topLayout->addWidget(descLabel, 1);
        topLayout->addWidget(amountLabel);

        // Bottom Row: Due Date + Status + Action
        QHBoxLayout* bottomLayout = new QHBoxLayout();

        QLabel* dueLabel = new QLabel("Due: " + dueStr);
        dueLabel->setStyleSheet("color: #94a3b8; font-size: 13px;");

        QLabel* statusLabel = new QLabel(status.toUpper());
        statusLabel->setAlignment(Qt::AlignCenter);

        if (status == "paid") {
            statusLabel->setStyleSheet("color: #22c55e; font-weight: 600; padding: 2px 10px; background: #052e16; border-radius: 4px;");
        }
        else {
            statusLabel->setStyleSheet("color: #eab308; font-weight: 600; padding: 2px 10px; background: #451a03; border-radius: 4px;");
        }

        bottomLayout->addWidget(dueLabel);
        bottomLayout->addStretch();
        bottomLayout->addWidget(statusLabel);

        mainLayout->addLayout(topLayout);
        mainLayout->addLayout(bottomLayout);

        // Add to List
        QListWidgetItem* item = new QListWidgetItem(ui.billsList);
        item->setData(Qt::UserRole, billId);
        item->setSizeHint(QSize(0, 105));
        ui.billsList->setItemWidget(item, card);
    }
}

void DashboardPage::onCreateNewBillClicked()
{
    bool ok;
    QString description = QInputDialog::getText(this, "New Bill",
        "Bill Description (Electricity, Water, etc.):", QLineEdit::Normal, "", &ok);
    if (!ok || description.trimmed().isEmpty()) return;

    double amount = QInputDialog::getDouble(this, "New Bill",
        "Amount (₹):", 500.0, 10.0, 100000.0, 2, &ok);
    if (!ok) return;

    QDateTime dueDate = QDateTime::currentDateTime().addDays(7); // Default 7 days

    int userId = AppState::instance().userId().toInt();

    QSqlQuery query = DatabaseManager::instance().executeQuery(R"(
        INSERT INTO bills (user_id, description, amount, due_date, status)
        VALUES (?, ?, ?, ?, 'pending')
    )", { userId, description.trimmed(), amount, dueDate });

    if (query.numRowsAffected() > 0) {
        showSuccessMessage("Bill Created", "New bill has been added successfully.");
        loadRealDataFromDB();   // Refresh bills list
    }
    else {
        showErrorMessage("Error", "Failed to create bill.");
    }
}
void DashboardPage::showSuccessMessage(const QString& title, const QString& message)
{
    QMessageBox::information(this, title, message);
}

void DashboardPage::showErrorMessage(const QString& title, const QString& message)
{
    QMessageBox::warning(this, title, message);
}