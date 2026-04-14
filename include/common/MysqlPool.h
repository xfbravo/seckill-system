/**
 * @file MysqlPool.h
 * @brief MySQL 连接池实现
 *
 * 使用 std::vector 维护连接，std::mutex + std::condition_variable 实现获取/归还
 */

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>
#include <mysql/mysql.h>

namespace seckill
{

class MysqlPool
{
public:
    using Ptr = std::shared_ptr<MysqlPool>;

    MysqlPool(const std::string& host, const std::string& user,
              const std::string& pass, const std::string& db,
              int port, int poolSize = 10);
    ~MysqlPool();

    // 获取一个连接
    MYSQL* get();

    // 归还一个连接
    void release(MYSQL* conn);

    // 关闭所有连接
    void close();

private:
    struct ConnInfo {
        MYSQL* conn;
        bool inUse;
    };
    std::vector<ConnInfo> pool_;
    std::mutex mutex_;
    std::condition_variable cv_;
    int poolSize_;
    std::string host_;
    std::string user_;
    std::string pass_;
    std::string db_;
    int port_;
    bool closed_ = false;
};

} // namespace seckill