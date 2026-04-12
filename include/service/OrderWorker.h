/**
 * @file OrderWorker.h
 * @brief 订单处理Worker - 后台异步处理秒杀订单
 *
 * 从Redis队列消费订单，写入MySQL
 */

#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <string>
#include <repository/OrderRepository.h>
#include <repository/ActivityRepository.h>

struct redisContext;

namespace seckill
{

class OrderWorker
{
public:
    using Ptr = std::shared_ptr<OrderWorker>;

    OrderWorker();
    ~OrderWorker();

    void start();
    void stop();

private:
    void run();
    bool processOrder(const std::string& orderJson);

    struct Impl;
    Impl* pImpl_;
    std::atomic<bool> running_;
    std::thread thread_;
};

} // namespace seckill
