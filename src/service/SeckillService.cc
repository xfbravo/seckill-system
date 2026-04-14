/**
 * @file SeckillService.cc
 * @brief 秒杀服务层实现 - 核心业务逻辑
 */

#include "service/SeckillService.h"
#include "service/StockService.h"
#include "service/ActivityService.h"
#include "service/OrderService.h"
#include "common/ErrorCode.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>

namespace seckill
{

struct SeckillService::Impl
{
    ActivityService::Ptr activityService;
    StockService::Ptr stockService;
    OrderService::Ptr orderService;
};

SeckillService::SeckillService()
{
    pImpl_ = new Impl();
    pImpl_->activityService = std::make_shared<ActivityService>();
    pImpl_->stockService = std::make_shared<StockService>();
    pImpl_->orderService = std::make_shared<OrderService>();
}

SeckillService::~SeckillService()
{
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

        // 2. 生成订单号
        std::string orderNo = generateOrderNo(activityId, userId);

        // 3. 创建订单 JSON
        Json::Value orderJson;
        orderJson["order_no"] = orderNo;
        orderJson["activity_id"] = (int64_t)activityId;
        orderJson["user_id"] = (int64_t)userId;
        orderJson["quantity"] = quantity;
        orderJson["status"] = 0;

        Json::StreamWriterBuilder builder;
        builder["indentation"] = "";
        std::string orderStr = Json::writeString(builder, orderJson);

        // 4. 原子执行：检查用户 + 扣库存 + 写队列 + 标记用户
        auto result = pImpl_->stockService->executeSeckill(activityId, userId, quantity, orderStr);
        if (result->code() != 0) {
            return result;
        }

        // 5. 返回结果
        Json::Value data;
        data["order_no"] = orderNo;
        data["status"] = "pending";
        data["remain_stock"] = result->data()["remain_stock"];

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
