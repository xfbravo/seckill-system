/**
 * @file OrderWorker.cc
 * @brief 订单处理Worker实现 - 多线程 + 批量写入
 * 每个线程独立的 Redis 连接，避免线程竞争
 */

#include "service/OrderWorker.h"
#include "repository/OrderRepository.h"
#include "repository/ActivityRepository.h"
#include "common/RedisKeys.h"
#include "model/Order.h"
#include <iostream>
#include <hiredis/hiredis.h>
#include <json/json.h>
#include <sstream>
#include <chrono>

namespace seckill
{

struct OrderWorker::Impl
{
    // 每个线程独立的 Redis 连接
    std::vector<redisContext*> threadRedis;
    OrderRepository::Ptr orderRepo;
    ActivityRepository::Ptr activityRepo;

    // 批量处理相关
    std::mutex batchMutex;
    std::condition_variable batchCv;
    std::vector<std::pair<std::string, Json::Value>> batchOrders;
    std::chrono::steady_clock::time_point lastFlush;

    static constexpr int WORKER_COUNT = 4;
    static constexpr int BATCH_SIZE = 50;
    static constexpr int BATCH_TIMEOUT_MS = 100;
};

OrderWorker::OrderWorker()
    : running_(false)
    , pImpl_(new Impl())
{
    // 初始化每个线程的 Redis 连接
    for (int i = 0; i < Impl::WORKER_COUNT; ++i) {
        redisContext* ctx = redisConnect("127.0.0.1", 6379);
        if (!ctx || ctx->err) {
            if (ctx) {
                std::cerr << "OrderWorker Redis connection error: " << ctx->errstr << std::endl;
                redisFree(ctx);
                ctx = nullptr;
            } else {
                std::cerr << "OrderWorker: Failed to allocate Redis connection for thread " << i << std::endl;
            }
        }
        pImpl_->threadRedis.push_back(ctx);
    }

    pImpl_->orderRepo = std::make_shared<OrderRepository>();
    pImpl_->activityRepo = std::make_shared<ActivityRepository>();
    pImpl_->lastFlush = std::chrono::steady_clock::now();
}

OrderWorker::~OrderWorker()
{
    stop();
    for (auto ctx : pImpl_->threadRedis) {
        if (ctx) {
            redisFree(ctx);
        }
    }
    delete pImpl_;
}

void OrderWorker::start()
{
    if (running_) return;

    running_ = true;
    for (int i = 0; i < Impl::WORKER_COUNT; ++i) {
        threads_.emplace_back(&OrderWorker::workerThread, this, i);
    }
    std::cout << "OrderWorker started with " << Impl::WORKER_COUNT << " threads" << std::endl;
}

void OrderWorker::stop()
{
    if (!running_) return;

    running_ = false;

    // 唤醒所有等待中的线程
    pImpl_->batchCv.notify_all();

    for (auto& t : threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
    threads_.clear();

    std::cout << "OrderWorker stopped" << std::endl;
}

void OrderWorker::workerThread(int threadId)
{
    // 每个线程使用自己的 Redis 连接
    redisContext* redis = pImpl_->threadRedis[threadId];

    while (running_) {
        if (!redis) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        // 使用 BRPOP 阻塞等待，1秒超时
        redisReply* reply = (redisReply*)redisCommand(
            redis,
            "BRPOP %s 1",
            RedisKeys::ordersPendingKey().c_str()
        );

        if (!reply) {
            continue;
        }

        if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
            std::string orderJson = reply->element[1]->str;
            processOrder(orderJson);
        }

        freeReplyObject(reply);

        // 检查是否需要强制刷新批次（防止超时）
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - pImpl_->lastFlush).count();
        if (elapsed > BATCH_TIMEOUT_MS) {
            flushBatch();
        }
    }
}

bool OrderWorker::processOrder(const std::string& orderJson)
{
    try {
        Json::Value json;
        std::istringstream iss(orderJson);
        Json::CharReaderBuilder builder;
        std::string errs;
        if (!Json::parseFromStream(builder, iss, &json, &errs)) {
            std::cerr << "OrderWorker: Failed to parse order JSON: " << errs << std::endl;
            return false;
        }

        // 加入批次
        {
            std::lock_guard<std::mutex> lock(pImpl_->batchMutex);
            pImpl_->batchOrders.push_back({orderJson, json});

            if ((int)pImpl_->batchOrders.size() >= BATCH_SIZE) {
                flushBatch();
            }
        }

        return true;

    } catch (const std::exception& e) {
        std::cerr << "OrderWorker: Exception processing order: " << e.what() << std::endl;
        return false;
    }
}

void OrderWorker::flushBatch()
{
    std::vector<std::pair<std::string, Json::Value>> ordersToProcess;

    {
        std::lock_guard<std::mutex> lock(pImpl_->batchMutex);
        if (pImpl_->batchOrders.empty()) {
            return;
        }
        ordersToProcess = std::move(pImpl_->batchOrders);
        pImpl_->batchOrders.clear();
        pImpl_->lastFlush = std::chrono::steady_clock::now();
    }

    // 构建订单列表并批量写入
    std::vector<Order> orderList;
    orderList.reserve(ordersToProcess.size());
    for (const auto& pair : ordersToProcess) {
        const Json::Value& json = pair.second;
        Order order;
        order.setOrderNo(json["order_no"].asString());
        order.setActivityId(json["activity_id"].asInt64());
        order.setUserId(json["user_id"].asInt64());
        order.setQuantity(json["quantity"].asInt());
        order.setStatus(1);
        orderList.push_back(order);
    }

    // 批量插入所有订单
    if (!pImpl_->orderRepo->createBatch(orderList)) {
        std::cerr << "OrderWorker: Batch create orders failed, count=" << orderList.size() << std::endl;
    } else {
        // 批量更新活动库存
        for (const auto& pair : ordersToProcess) {
            const Json::Value& json = pair.second;
            pImpl_->activityRepo->updateRemainStock(
                json["activity_id"].asInt64(), json["remain_stock"].asInt());
        }
        std::cout << "OrderWorker: Batch processed " << orderList.size() << " orders" << std::endl;
    }
}

} // namespace seckill