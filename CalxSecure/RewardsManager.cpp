#include "RewardsManager.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QRandomGenerator>
#include <QDateTime>

RewardsManager::RewardsManager(QObject* parent) : QObject(parent)
{
    initDummyRewards();
}

void RewardsManager::initDummyRewards()
{
    // Run once on first launch
    QSqlQuery check("SELECT COUNT(*) FROM rewards");
    if (check.next() && check.value(0).toInt() == 0) {
        QSqlQuery q;
        q.exec(R"(
            INSERT INTO rewards (title, amount, description, type) VALUES
            ('Daily Login Bonus', 0.50, 'Log in daily', 'daily'),
            ('Scratch & Win', 2.50, 'Scratch to reveal', 'scratch'),
            ('First Recharge Bonus', 3.00, 'After your first recharge', 'cashback'),
            ('Referral Reward', 5.00, 'Invite a friend', 'referral'),
            ('Mini Cashback', 1.00, 'On any transaction', 'cashback'),
            ('Lucky Scratch', 4.00, 'Limited time', 'scratch'),
            ('Weekend Bonus', 1.50, 'Weekend special', 'daily'),
            ('Transaction Streak', 2.00, '5+ transactions', 'cashback')
        )");
    }
}

QVector<Reward> RewardsManager::getAvailableRewards(int userId)
{
    QVector<Reward> rewards;
    QSqlQuery q(R"(
        SELECT r.id, r.title, r.amount, r.description, r.type, 
               COALESCE(ur.claimed, 0) as claimed
        FROM rewards r
        LEFT JOIN user_rewards ur ON r.id = ur.reward_id AND ur.user_id = ?
        WHERE COALESCE(ur.claimed, 0) = 0
    )");
    q.addBindValue(userId);
    q.exec();

    while (q.next()) {
        Reward r;
        r.id = q.value(0).toInt();
        r.title = q.value(1).toString();
        r.amount = q.value(2).toDouble();
        r.description = q.value(3).toString();
        r.type = q.value(4).toString();
        r.isClaimed = q.value(5).toBool();
        rewards.append(r);
    }
    return rewards;
}

bool RewardsManager::claimReward(int userId, int rewardId, double& amountAwarded)
{
    QSqlQuery q("SELECT amount FROM rewards WHERE id = ?");
    q.addBindValue(rewardId);
    if (!q.exec() || !q.next()) return false;

    amountAwarded = q.value(0).toDouble();

    // Add to user balance
    DatabaseManager::instance().executeQuery(
        "UPDATE users SET balance = balance + ? WHERE id = ?", { amountAwarded, userId });

    // Mark as claimed
    DatabaseManager::instance().executeQuery(
        "INSERT INTO user_rewards (user_id, reward_id, claimed_at) VALUES (?, ?, NOW())",
        { userId, rewardId });

    emit rewardClaimed(amountAwarded);
    return true;
}

double RewardsManager::getTotalRewards(int userId)
{
    QSqlQuery q("SELECT SUM(amount) FROM user_rewards ur JOIN rewards r ON ur.reward_id = r.id WHERE ur.user_id = ?");
    q.addBindValue(userId);
    return q.next() ? q.value(0).toDouble() : 0.0;
}