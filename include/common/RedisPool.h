/**
 * @file RedisPool.h
 * @brief Redis 连接池实现
 *
 * 使用 std::vector 维护连接，std::mutex + std::condition_variable 实现获取/归还
 */

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>
#include <hiredis/hiredis.h>

namespace seckill
{

class RedisPool
{
public:
    using Ptr = std::shared_ptr<RedisPool>;

    RedisPool(const std::string& host, int port, int poolSize = 10);
    ~RedisPool();

    // 获取一个连接
    redisContext* get();

    // 归还一个连接
    void release(redisContext* conn);

    // 关闭所有连接
    void close();

private:
    std::vector<redisContext*> pool_;
    std::mutex mutex_;
    std::condition_variable cv_;
    int poolSize_;
    std::string host_;
    int port_;
    bool closed_ = false;
};

} // namespace seckill