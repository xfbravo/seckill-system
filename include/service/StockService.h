/**
 * @file StockService.h
 * @brief 库存服务层
 */

#pragma once

#include <memory>
#include "common/Result.h"

namespace seckill
{

class StockService
{
public:
    using Ptr = std::shared_ptr<StockService>;

    StockService();
    ~StockService();

    Result::Ptr initStock(long long activityId, int stock);
    Result::Ptr decreaseStock(long long activityId, int quantity);
    Result::Ptr getStock(long long activityId);
    Result::Ptr increaseStock(long long activityId, int quantity);

    // 原子秒杀操作：检查用户 + 扣减库存 + 写入队列 + 标记用户已购买
    // @return code=0成功, code>0失败; data["remain_stock"]包含剩余库存
    Result::Ptr executeSeckill(long long activityId, long long userId, int quantity,
                               const std::string& orderJson);

private:
    struct Impl;
    Impl* pImpl_;
};

} // namespace seckill
