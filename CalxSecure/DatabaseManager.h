
// STEP 2: DATABASE SINGLETON (Production-Grade)
// ================================================
// File: db/DatabaseManager.h
// Location: Place inside your project root → /db/DatabaseManager.h

#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMutex>
#include <QMutexLocker>
#include <QVariantList>
#include <QDebug>
#include <QDateTime>

class DatabaseManager
{
    Q_DISABLE_COPY(DatabaseManager)
public:
    static DatabaseManager& instance();

    // Connect to PostgreSQL (call once at app startup)
    bool connect();

    // Execute any query with prepared statements (thread-safe)
    QSqlQuery executeQuery(const QString& sql, const QVariantList& params = {});

    // Convenience methods for common operations
    bool transactionBegin();
    bool transactionCommit();
    bool transactionRollback();

    // Check if connected and healthy
    bool isHealthy() const;

private:
    DatabaseManager();
    ~DatabaseManager();

    QSqlDatabase m_db;
    mutable QMutex m_mutex;
};