// ================================================
// STEP 2: DATABASE SINGLETON (Implementation)
// ================================================
// File: db/DatabaseManager.cpp
// Location: Place inside your project root → /db/DatabaseManager.cpp

#include "DatabaseManager.h"
#include <QCoreApplication>
#include <QStandardPaths>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager singleton;
    return singleton;
}

DatabaseManager::DatabaseManager()
{
    // Ensure PostgreSQL driver is available
    if (!QSqlDatabase::drivers().contains("QPSQL")) {
        qCritical() << "Qt PostgreSQL driver (QPSQL) is not available. "
            << "Please rebuild Qt with PostgreSQL support or install the driver.";
    }
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::connect()
{
    QMutexLocker locker(&m_mutex);

    if (m_db.isOpen()) {
        return true;
    }

    m_db = QSqlDatabase::addDatabase("QPSQL", "calxsecure_main");
    m_db.setHostName("localhost");           // Change in production via config
    m_db.setPort(5432);
    m_db.setDatabaseName("calxsecure");
    m_db.setUserName("calxsecure_user");     // Use a dedicated DB user (least privilege)
    m_db.setPassword("CalxSecure2026!");     // ← In production: load from QSettings + encrypted vault / env

    if (!m_db.open()) {
        qCritical() << "Database connection failed:" << m_db.lastError().text();
        return false;
    }

    qInfo() << "✅ PostgreSQL connected successfully (CalxSecure)";

    // Optional: Set application_name for monitoring
    QSqlQuery query(m_db);
    query.exec("SET application_name = 'CalxSecure-Desktop'");

    return true;
}

QSqlQuery DatabaseManager::executeQuery(const QString& sql, const QVariantList& params)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query(m_db);
    query.prepare(sql);

    for (int i = 0; i < params.size(); ++i) {
        query.bindValue(i, params[i]);
    }

    if (!query.exec()) {
        qCritical() << "Query failed:" << query.lastError().text()
            << "\nSQL:" << sql
            << "\nParams:" << params;
    }

    return query;
}

bool DatabaseManager::transactionBegin()
{
    QMutexLocker locker(&m_mutex);
    return m_db.transaction();
}

bool DatabaseManager::transactionCommit()
{
    QMutexLocker locker(&m_mutex);
    return m_db.commit();
}

bool DatabaseManager::transactionRollback()
{
    QMutexLocker locker(&m_mutex);
    return m_db.rollback();
}

bool DatabaseManager::isHealthy() const
{
    QMutexLocker locker(&m_mutex);
    return m_db.isOpen() && m_db.isValid();
}