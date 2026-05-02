#include "RechargeManager.h"

RechargeManager::RechargeManager(QObject* parent) : QObject(parent)
{
    loadMockData();
}

void RechargeManager::loadMockData()
{
    m_operators = {
        {1, "Jio", "JIO"},
        {2, "Airtel", "AIR"},
        {3, "Vi", "VI"},
        {4, "BSNL", "BSNL"}
    };
}

QVector<Operator> RechargeManager::getOperators()
{
    return m_operators;
}

QVector<RechargePlan> RechargeManager::getPlans(const QString& operatorCode)
{
    QVector<RechargePlan> plans;

    if (operatorCode == "JIO") {
        plans << RechargePlan{ 1, "Jio 2GB/Day", 239, "2GB/day + Unlimited calls", "28 Days", "2GB/day", "", "data", true };
        plans << RechargePlan{ 2, "Jio Unlimited", 299, "Unlimited data + calls", "28 Days", "Unlimited", "", "combo", true };
        plans << RechargePlan{ 3, "Jio Talktime", 100, "₹100 Talktime", "28 Days", "", "₹100", "talktime", false };
    }
    else if (operatorCode == "AIR") {
        plans << RechargePlan{ 4, "Airtel 3GB/Day", 359, "3GB/day + Unlimited", "28 Days", "3GB/day", "", "data", true };
        plans << RechargePlan{ 5, "Airtel Combo", 199, "1.5GB + calls", "28 Days", "1.5GB", "", "combo", false };
    }

    return plans;
}