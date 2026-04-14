/**
 * @file SeckillService.h
 * @brief 秒杀服务层 - 核心业务逻辑
 */

#pragma once

#include <memory>
#include <string>
#include "common/Result.h"

namespace seckill
{

class SeckillService
{
public:
    using Ptr = std::shared_ptr<SeckillService>;

    SeckillService();
    ~SeckillService();

    Result::Ptr executeSeckill(long long activityId, long long userId, int quantity = 1);
    Result::Ptr getSeckillResult(const std::string& orderNo);

private:
    int checkActivityStatus(long long activityId);
    std::string generateOrderNo(long long activityId, long long userId);

    struct Impl;
    Impl* pImpl_;
};

} // namespace seckill
