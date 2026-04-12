/**
 * @file OrderWorker.cc
 * @brief 订单处理Worker实现
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

namespace seckill
{

struct OrderWorker::Impl
{
    redisContext* redis = nullptr;
    OrderRepository::Ptr orderRepo;
    ActivityRepository::Ptr activityRepo;
};

OrderWorker::OrderWorker()
    : running_(false)
    , pImpl_(new Impl())
{
    // 连接Redis
    pImpl_->redis = redisConnect("127.0.0.1", 6379);
    if (!pImpl_->redis || pImpl_->redis->err) {
        if (pImpl_->redis) {
            std::cerr << "OrderWorker Redis connection error: " << pImpl_->redis->errstr << std::endl;
            redisFree(pImpl_->redis);
            pImpl_->redis = nullptr;
        } else {
            std::cerr << "OrderWorker: Failed to allocate Redis connection" << std::endl;
        }
    }

    pImpl_->orderRepo = std::make_shared<OrderRepository>();
    pImpl_->activityRepo = std::make_shared<ActivityRepository>();
}

OrderWorker::~OrderWorker()
{
    stop();
    if (pImpl_->redis) {
        redisFree(pImpl_->redis);
    }
    delete pImpl_;
}

void OrderWorker::start()
{
    if (running_) return;

    running_ = true;
    thread_ = std::thread(&OrderWorker::run, this);
    std::cout << "OrderWorker started" << std::endl;
}

void OrderWorker::stop()
{
    if (!running_) return;

    running_ = false;
    if (pImpl_->redis) {
        redisFree(pImpl_->redis);
        pImpl_->redis = nullptr;
    }
    if (thread_.joinable()) {
        thread_.join();
    }
    std::cout << "OrderWorker stopped" << std::endl;
}

void OrderWorker::run()
{
    while (running_) {
        if (!pImpl_->redis) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        // BRPOP - 阻塞等待队列中的订单
        redisReply* reply = (redisReply*)redisCommand(
            pImpl_->redis,
            "BRPOP %s 1",  // 1秒超时
            RedisKeys::ordersPendingKey().c_str()
        );

        if (!reply) {
            continue;
        }

        if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
            // reply->element[1] 是订单JSON
            std::string orderJson = reply->element[1]->str;
            processOrder(orderJson);
        }

        freeReplyObject(reply);
    }
}

bool OrderWorker::processOrder(const std::string& orderJson)
{
    try {
        // 解析订单JSON
        Json::Value json;
        std::istringstream iss(orderJson);
        Json::CharReaderBuilder builder;
        std::string errs;
        if (!Json::parseFromStream(builder, iss, &json, &errs)) {
            std::cerr << "OrderWorker: Failed to parse order JSON: " << errs << std::endl;
            return false;
        }

        // 构建Order对象
        Order order;
        order.setOrderNo(json["order_no"].asString());
        order.setActivityId(json["activity_id"].asInt64());
        order.setUserId(json["user_id"].asInt64());
        order.setQuantity(json["quantity"].asInt());
        order.setStatus(1); // 已确认

        // 写入MySQL订单表
        if (!pImpl_->orderRepo->create(order)) {
            std::cerr << "OrderWorker: Failed to create order in MySQL: " << order.getOrderNo() << std::endl;
            return false;
        }

        // 更新MySQL活动库存
        if (!pImpl_->activityRepo->updateRemainStock(order.getActivityId(), json["remain_stock"].asInt())) {
            std::cerr << "OrderWorker: Failed to update activity stock" << std::endl;
        }

        std::cout << "OrderWorker: Processed order " << order.getOrderNo() << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "OrderWorker: Exception processing order: " << e.what() << std::endl;
        return false;
    }
}

} // namespace seckill
