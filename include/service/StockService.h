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

private:
    struct Impl;
    Impl* pImpl_;
};

} // namespace seckill
