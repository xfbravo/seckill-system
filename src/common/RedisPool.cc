/**
 * @file RedisPool.cc
 * @brief Redis 连接池实现
 */

#include "common/RedisPool.h"
#include <iostream>

namespace seckill
{

RedisPool::RedisPool(const std::string& host, int port, int poolSize)
    : poolSize_(poolSize), host_(host), port_(port)
{
    for (int i = 0; i < poolSize_; ++i) {
        redisContext* ctx = redisConnect(host.c_str(), port);
        if (!ctx || ctx->err) {
            if (ctx) {
                std::cerr << "RedisPool: connection error: " << ctx->errstr << std::endl;
                redisFree(ctx);
            } else {
                std::cerr << "RedisPool: Failed to allocate connection" << std::endl;
            }
            continue;
        }
        pool_.push_back(ctx);
    }
}

RedisPool::~RedisPool()
{
    close();
}

redisContext* RedisPool::get()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !pool_.empty() || closed_; });

    if (closed_) return nullptr;

    redisContext* conn = pool_.back();
    pool_.pop_back();
    return conn;
}

void RedisPool::release(redisContext* conn)
{
    if (!conn) return;

    std::lock_guard<std::mutex> lock(mutex_);
    if (closed_) {
        redisFree(conn);
        return;
    }

    // 检查连接是否有效
    if (conn->err) {
        std::cerr << "RedisPool: released broken connection: " << conn->errstr << std::endl;
        redisFree(conn);
        // 创建新连接替换
        conn = redisConnect(host_.c_str(), port_);
        if (!conn || conn->err) {
            if (conn) redisFree(conn);
            return;
        }
    }

    pool_.push_back(conn);
    cv_.notify_one();
}

void RedisPool::close()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (closed_) return;
    closed_ = true;

    for (auto conn : pool_) {
        redisFree(conn);
    }
    pool_.clear();
    cv_.notify_all();
}

} // namespace seckill