/**
 * @file SeckillService.cc
 * @brief 秒杀服务层实现 - 核心业务逻辑
 */

#include "service/SeckillService.h"
#include "service/StockService.h"
#include "service/ActivityService.h"
#include "service/OrderService.h"
#include "common/ErrorCode.h"
#include "common/RedisKeys.h"
#include "common/Config.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <hiredis/hiredis.h>
#include <ctime>
#include <chrono>

namespace seckill
{

struct SeckillService::Impl
{
    ActivityService::Ptr activityService;
    StockService::Ptr stockService;
    OrderService::Ptr orderService;
    redisContext* redis;
};

SeckillService::SeckillService()
{
    pImpl_ = new Impl();
    pImpl_->activityService = std::make_shared<ActivityService>();
    pImpl_->stockService = std::make_shared<StockService>();
    pImpl_->orderService = std::make_shared<OrderService>();

    Config& cfg = Config::instance();
    pImpl_->redis = redisConnect(cfg.redisHost().c_str(), cfg.redisPort());
    if (pImpl_->redis && pImpl_->redis->err) {
        std::cerr << "Redis connection error: " << pImpl_->redis->errstr << std::endl;
        redisFree(pImpl_->redis);
        pImpl_->redis = nullptr;
    }
}

SeckillService::~SeckillService()
{
    if (pImpl_->redis) {
        redisFree(pImpl_->redis);
        pImpl_->redis = nullptr;
    }
    delete pImpl_;
}

Result::Ptr SeckillService::executeSeckill(long long activityId, long long userId, int quantity)
{
    try {
        // 1. 检查活动状态
        int status = checkActivityStatus(activityId);
        if (status < 0) {
            return Result::fail(ErrorCode::ERR_ACTIVITY_NOT_FOUND, "Activity not found");
        }
        if (status == 0) {  // NOT_START
            return Result::fail(ErrorCode::ERR_ACTIVITY_NOT_START, "Activity has not started");
        }
        if (status == 2) {  // ENDED
            return Result::fail(ErrorCode::ERR_ACTIVITY_ENDED, "Activity has ended");
        }

        // 2. 检查用户是否已经购买过
        if (pImpl_->redis) {
            std::string buyKey = RedisKeys::userBuyKey(activityId, userId);
            redisReply* reply = (redisReply*)redisCommand(pImpl_->redis, "EXISTS %s", buyKey.c_str());
            if (reply && reply->type == REDIS_REPLY_INTEGER && reply->integer == 1) {
                freeReplyObject(reply);
                return Result::fail(ErrorCode::ERR_PARAM_INVALID, "You have already purchased this item");
            }
            if (reply) freeReplyObject(reply);
        }

        // 3. 原子扣减库存
        auto decreaseResult = pImpl_->stockService->decreaseStock(activityId, quantity);
        if (decreaseResult->code() != 0) {
            return Result::fail(ErrorCode::ERR_ACTIVITY_SOLD_OUT, "Stock is sold out");
        }

        // 4. 生成订单号
        std::string orderNo = generateOrderNo(activityId, userId);

        // 5. 创建订单 JSON
        Json::Value orderJson;
        orderJson["order_no"] = orderNo;
        orderJson["activity_id"] = (int64_t)activityId;
        orderJson["user_id"] = (int64_t)userId;
        orderJson["quantity"] = quantity;
        orderJson["status"] = 0;
        orderJson["remain_stock"] = decreaseResult->data()["remain_stock"]; // 扣减后的库存

        // 6. 写入订单队列
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "";
        std::string orderStr = Json::writeString(builder, orderJson);

        if (!pushToOrderQueue(orderStr)) {
            pImpl_->stockService->increaseStock(activityId, quantity);
            return Result::fail(ErrorCode::ERR_ORDER_CREATE_FAIL, "Failed to create order");
        }

        // 7. 标记用户已购买
        if (pImpl_->redis) {
            std::string buyKey = RedisKeys::userBuyKey(activityId, userId);
            redisCommand(pImpl_->redis, "SETEX %s 86400 1", buyKey.c_str());
        }

        // 8. 返回结果
        Json::Value data;
        data["order_no"] = orderNo;
        data["status"] = "pending";
        data["remain_stock"] = decreaseResult->data()["remain_stock"];

        return Result::success("Seckill successful, order is being processed", data);

    } catch (const std::exception& e) {
        std::cerr << "executeSeckill exception: " << e.what() << std::endl;
        return Result::fail(ErrorCode::ERR_INTERNAL, e.what());
    }
}

Result::Ptr SeckillService::getSeckillResult(const std::string& orderNo)
{
    return pImpl_->orderService->getOrderByNo(orderNo);
}

int SeckillService::checkActivityStatus(long long activityId)
{
    auto result = pImpl_->activityService->checkActivityStatus(activityId);
    if (result->code() != 0) {
        return -1;
    }
    return result->data()["status"].asInt();
}

bool SeckillService::pushToOrderQueue(const std::string& orderJson)
{
    if (!pImpl_->redis) return false;

    redisReply* reply = (redisReply*)redisCommand(
        pImpl_->redis,
        "LPUSH %s %s",
        RedisKeys::ordersPendingKey().c_str(),
        orderJson.c_str()
    );

    if (!reply) return false;

    bool success = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
    freeReplyObject(reply);
    return success;
}

std::string SeckillService::generateOrderNo(long long activityId, long long userId)
{
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();

    std::stringstream ss;
    ss << timestamp;
    ss << std::setw(4) << std::setfill('0') << activityId;
    ss << std::setw(4) << std::setfill('0') << userId;
    ss << std::setw(4) << std::setfill('0') << (rand() % 10000);

    return ss.str();
}

} // namespace seckill
