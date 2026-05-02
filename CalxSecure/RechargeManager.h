#ifndef RECHARGEMANAGER_H
#define RECHARGEMANAGER_H

#include <QObject>
#include <QVector>
#include <QString>

struct Operator {
    int id;
    QString name;
    QString code;
};

struct RechargePlan {
    int id;
    QString name;
    double amount;
    QString description;
    QString validity;
    QString data;
    QString talktime;
    QString category; // "data", "talktime", "combo"
    bool isPopular = false;
};

class RechargeManager : public QObject
{
    Q_OBJECT
public:
    explicit RechargeManager(QObject* parent = nullptr);

    QVector<Operator> getOperators();
    QVector<RechargePlan> getPlans(const QString& operatorCode);

signals:
    void plansFetched(const QVector<RechargePlan>& plans);
    void errorOccurred(const QString& message);

private:
    void loadMockData();
    QVector<Operator> m_operators;
};

#endif