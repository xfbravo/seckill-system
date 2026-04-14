/**
 * @file OrderWorker.h
 * @brief 订单处理Worker - 后台异步处理秒杀订单
 *
 * 从Redis队列消费订单，写入MySQL
 * 支持多线程并发处理 + 批量写入
 */

#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
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

    // 多线程处理
    void workerThread(int threadId);
    void flushBatch();

    struct Impl;
    Impl* pImpl_;
    std::atomic<bool> running_;
    std::vector<std::thread> threads_;

    // 批量处理
    static constexpr int BATCH_SIZE = 50;
    static constexpr int BATCH_TIMEOUT_MS = 100;
};

} // namespace seckill