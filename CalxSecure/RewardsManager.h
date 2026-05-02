#ifndef REWARDSMANAGER_H
#define REWARDSMANAGER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QDateTime>

struct Reward {
    int id;
    QString title;
    double amount;           // ₹0.00 to ₹5.00
    QString description;
    QString type;            // "scratch", "daily", "cashback", "referral"
    bool isClaimed = false;
    QDateTime expiresAt;
};

class RewardsManager : public QObject
{
    Q_OBJECT
public:
    explicit RewardsManager(QObject* parent = nullptr);

    QVector<Reward> getAvailableRewards(int userId);
    bool claimReward(int userId, int rewardId, double& amountAwarded);
    double getTotalRewards(int userId);

    void generateDailyReward(int userId);
    void createScratchCard(int userId);

signals:
    void rewardClaimed(double amount);

private:
    void initDummyRewards();
    void createUserRewardEntry(int userId, const Reward& reward);
};

#endif