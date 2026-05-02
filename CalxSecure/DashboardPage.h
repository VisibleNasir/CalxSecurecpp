#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include <QWidget>
#include "ui_DashboardPage.h"
#include "components/LoadingOverlay.h"
#include "RechargeManager.h"   // Add this

struct RechargePlan;   // Forward declaration

class DashboardPage : public QWidget
{
    Q_OBJECT
public:
    explicit DashboardPage(QWidget* parent = nullptr);
    ~DashboardPage();

signals:
    void logoutRequested();
    void homeRequested();

public slots:
    void updateBalance(double balance, const QString& fullName);
    void setCurrentTab(int index);

private slots:
    void onShowBalanceClicked();
    void onAddFundsConfirmClicked();
    void onPayNowClicked();
    void onSendMoneyClicked();
    void onLogoutClicked();

    // PIN Management
    void onChangeTransactionPinClicked();
    void onResetTransactionPinClicked();

    // Transfer / On-ramp
    void setupTransferPage();
    void onPaymentMethodChanged(int index);
    void loadOnrampHistory();

    // Recharge Full System
    void setupRechargePage();
    void onFetchRechargePlans();
    void onRechargePlanSelected(QListWidgetItem* item);
    void onRechargeProceedToPay();

    // Other existing slots
    void validateP2PInput();
    void evaluateSendButtonState();
    void onBillItemClicked(QListWidgetItem* item);
    void onCreateNewBillClicked();
    void refreshBalanceFromDB();
    void loadRealDataFromDB();
    void onScratchCardClicked();
private:
    bool verifyTransactionPin();
    bool verifyCurrentPinForReset();
    void loadBills(int userId);
    void showSuccessMessage(const QString& title, const QString& message);
    void showErrorMessage(const QString& title, const QString& message);
    void loadAvailableRewards(QListWidget* list);
private:
    Ui::DashboardPageClass ui;
    void setupRewardsPage();
    double currentBalance = 1000.0;
    bool balanceVisible = false;
    LoadingOverlay* m_loadingOverlay = nullptr;
    void setupSidebarNavigation();
    void setupSettingsPage();
    // On-ramp
    QListWidget* onrampHistoryList = nullptr;
    QString currentPaymentMethod;
    QString currentProvider;

    // Recharge
    RechargeManager* rechargeManager = nullptr;
    QLineEdit* rechargeNumberEdit = nullptr;
    QComboBox* rechargeOperatorCombo = nullptr;
    QListWidget* plansListWidget = nullptr;
    RechargePlan selectedRechargePlan;

    QString currentOperator;
};

#endif // DASHBOARDPAGE_H