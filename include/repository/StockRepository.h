/**
 * @file StockRepository.h
 * @brief 库存数据访问层
 *
 * 为什么这样设计？
 * - 封装 Redis 库存操作
 * - 使用 Lua 脚本保证原子性，避免超卖
 * - Repository 模式隔离数据访问逻辑
 */

#pragma once

#include <memory>
#include <string>
#include <hiredis/hiredis.h>
#include "common/RedisPool.h"

namespace seckill
{

class StockRepository
{
public:
    using Ptr = std::shared_ptr<StockRepository>;

    StockRepository();
    ~StockRepository();

    // 初始化库存
    bool initStock(long long activityId, int stock);

    // 扣减库存
    int decreaseStock(long long activityId, int quantity);

    // 查询库存
    int getStock(long long activityId);

    // 回补库存
    bool increaseStock(long long activityId, int quantity);

    // 原子秒杀操作：检查用户 + 扣减库存 + 写入队列 + 标记用户已购买
    int executeSeckill(long long activityId, long long userId, int quantity,
                       const std::string& orderJson, int buyExpireSec = 86400);

private:
    int executeScript(const std::string& script, long long activityId, int quantity);
    int executeSeckillScript(const std::string& script, long long activityId,
                            long long userId, int quantity, const std::string& orderJson,
                            int buyExpireSec);

    RedisPool::Ptr redisPool_;
    static const char* DECREASE_STOCK_SCRIPT;
};

} // namespace seckill
