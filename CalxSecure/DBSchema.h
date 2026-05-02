#pragma once
#include <QDebug>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>

class DBSchema {
public:
    static void initialize(QSqlDatabase& db) {
        QSqlQuery query(db);

        // ================= USERS =================
        execute(query, R"(
            CREATE TABLE IF NOT EXISTS users (
                id SERIAL PRIMARY KEY,
                username VARCHAR(50) UNIQUE NOT NULL,
                email VARCHAR(100) UNIQUE NOT NULL,
                password_hash TEXT NOT NULL,
                salt BYTEA NOT NULL,
                role VARCHAR(20) DEFAULT 'user',
                full_name VARCHAR(100),
                phone VARCHAR(20),
                balance DECIMAL(15,2) DEFAULT 1000.00,
                transaction_pin VARCHAR(10),
                is_active BOOLEAN DEFAULT true,
                last_login TIMESTAMP,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
        )");

        // ================= TRANSACTIONS =================
        execute(query, R"(
            CREATE TABLE IF NOT EXISTS transactions (
                id SERIAL PRIMARY KEY,
                from_user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,
                to_user_id INTEGER,
                type VARCHAR(50),
                amount DECIMAL(15,2) NOT NULL,
                recipient VARCHAR(100),
                description TEXT,
                status VARCHAR(20) DEFAULT 'completed',
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
        )");

        // ================= ONRAMP TRANSACTIONS =================
        execute(query, R"(
            CREATE TABLE IF NOT EXISTS onramp_transactions (
                id SERIAL PRIMARY KEY,
                user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,
                amount DECIMAL(15,2) NOT NULL,
                payment_method VARCHAR(50) NOT NULL,
                provider VARCHAR(100),
                status VARCHAR(20) DEFAULT 'pending',
                transaction_id VARCHAR(100),
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
        )");

        // ================= BILLS =================
        execute(query, R"(
            CREATE TABLE IF NOT EXISTS bills (
                id SERIAL PRIMARY KEY,
                user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,
                description VARCHAR(255),
                amount DECIMAL(15,2),
                due_date TIMESTAMP,
                status VARCHAR(20) DEFAULT 'pending',
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
        )");

        // ================= REWARDS (Master Table) =================
        execute(query, R"(
            CREATE TABLE IF NOT EXISTS rewards (
                id SERIAL PRIMARY KEY,
                title TEXT NOT NULL,
                amount REAL NOT NULL CHECK (amount BETWEEN 0 AND 5),
                description TEXT,
                type TEXT NOT NULL,          -- 'scratch', 'daily', 'cashback', 'referral'
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
        )");

        // ================= USER REWARDS (Claim History) =================
        execute(query, R"(
            CREATE TABLE IF NOT EXISTS user_rewards (
                id SERIAL PRIMARY KEY,
                user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,
                reward_id INTEGER REFERENCES rewards(id),
                amount REAL NOT NULL,
                claimed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
        )");

        // ================= RECHARGES =================
        execute(query, R"(
            CREATE TABLE IF NOT EXISTS recharges (
                id SERIAL PRIMARY KEY,
                user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,
                phone_number VARCHAR(20),
                amount DECIMAL(10,2),
                provider VARCHAR(50),
                status VARCHAR(20),
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
        )");

        // ================= SEED DATA =================
        execute(query, R"(
            INSERT INTO users (id, username, email, password_hash, salt, full_name, phone, balance, transaction_pin)
            VALUES (1, 'nasir', 'nasir@test.com', 'dummyhash', decode('73616c74','hex'),
                    'Nasir Nadaf', '9876543210', 5000.00, '1234')
            ON CONFLICT (id) DO NOTHING;
        )");

        // Seed On-ramp history
        execute(query, R"(
            INSERT INTO onramp_transactions (user_id, amount, payment_method, provider, status, created_at)
            VALUES
            (1, 2500.00, 'UPI', 'HDFC Bank', 'success', NOW() - INTERVAL '2 hours'),
            (1, 5000.00, 'CARD', 'Axis Bank', 'success', NOW() - INTERVAL '1 day'),
            (1, 1200.00, 'NETBANKING', 'SBI', 'failed', NOW() - INTERVAL '3 hours'),
            (1, 800.00, 'UPI', 'Kotak Bank', 'success', NOW() - INTERVAL '5 hours')
            ON CONFLICT DO NOTHING;
        )");

        // Seed Rewards (Master)
        execute(query, R"(
            INSERT INTO rewards (title, amount, description, type) VALUES
            ('Daily Login Bonus', 0.50, 'Log in every day', 'daily'),
            ('Scratch & Win', 2.50, 'Scratch to reveal reward', 'scratch'),
            ('First Recharge Bonus', 3.00, 'After your first recharge', 'cashback'),
            ('Referral Reward', 5.00, 'Invite a friend', 'referral'),
            ('Mini Cashback', 1.00, 'On any transaction', 'cashback'),
            ('Lucky Scratch', 4.00, 'Limited time offer', 'scratch'),
            ('Weekend Bonus', 1.50, 'Weekend special', 'daily'),
            ('Transaction Streak', 2.00, '5+ transactions this week', 'cashback')
            ON CONFLICT DO NOTHING;
        )");

        qInfo() << "✅ Database schema initialized with full fintech modules + Rewards System.";
    }

private:
    static void execute(QSqlQuery& q, const QString& sql) {
        if (!q.exec(sql)) {
            qWarning() << "Schema execution failed:" << q.lastError().text();
        }
    }
};