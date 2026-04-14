/**
 * @file MysqlPool.cc
 * @brief MySQL 连接池实现
 */

#include "common/MysqlPool.h"
#include <iostream>

namespace seckill
{

MysqlPool::MysqlPool(const std::string& host, const std::string& user,
                     const std::string& pass, const std::string& db,
                     int port, int poolSize)
    : poolSize_(poolSize), host_(host), user_(user), pass_(pass), db_(db), port_(port)
{
    for (int i = 0; i < poolSize_; ++i) {
        MYSQL* conn = mysql_init(nullptr);
        if (!conn) {
            std::cerr << "MysqlPool: Failed to init MySQL" << std::endl;
            continue;
        }

        if (!mysql_real_connect(conn, host.c_str(), user.c_str(), pass.c_str(),
                               db.c_str(), port, nullptr, 0)) {
            std::cerr << "MysqlPool: connection error: " << mysql_error(conn) << std::endl;
            mysql_close(conn);
            continue;
        }

        mysql_set_character_set(conn, "utf8mb4");
        pool_.push_back({conn, false});
    }
}

MysqlPool::~MysqlPool()
{
    close();
}

MYSQL* MysqlPool::get()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] {
        for (auto& ci : pool_) {
            if (!ci.inUse) return true;
        }
        return closed_;
    });

    if (closed_) return nullptr;

    for (auto& ci : pool_) {
        if (!ci.inUse) {
            ci.inUse = true;
            return ci.conn;
        }
    }
    return nullptr;
}

void MysqlPool::release(MYSQL* conn)
{
    if (!conn) return;

    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& ci : pool_) {
        if (ci.conn == conn) {
            ci.inUse = false;
            cv_.notify_one();
            return;
        }
    }
}

void MysqlPool::close()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (closed_) return;
    closed_ = true;

    for (auto& ci : pool_) {
        if (ci.conn) {
            mysql_close(ci.conn);
        }
    }
    pool_.clear();
    cv_.notify_all();
}

} // namespace seckill