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
#include <mutex>

namespace seckill
{

class StockRepository
{
public:
    using Ptr = std::shared_ptr<StockRepository>;

    StockRepository();
    ~StockRepository();

    // 初始化库存
    // @param activityId 活动ID
    // @param stock 库存数量
    // @return 成功返回 true
    bool initStock(long long activityId, int stock);

    // 扣减库存 (原子操作)
    // @param activityId 活动ID
    // @param quantity 购买数量
    // @return 成功返回剩余库存，失败返回 -1
    int decreaseStock(long long activityId, int quantity);

    // 查询库存
    // @param activityId 活动ID
    // @return 库存数量，失败返回 -1
    int getStock(long long activityId);

    // 回补库存 (用于取消订单等情况)
    // @param activityId 活动ID
    // @param quantity 回补数量
    // @return 成功返回 true
    bool increaseStock(long long activityId, int quantity);

private:
    // Redis 连接
    redisContext* redis_;
    std::mutex redisMutex_;  // Protects Redis operations

    // Lua 脚本：扣减库存
    // 逻辑：如果库存 >= 购买数量，则扣减成功
    // KEYS[1] = stock:{activity_id}
    // ARGV[1] = quantity
    static const char* DECREASE_STOCK_SCRIPT;

    // 执行 Lua 脚本
    int executeScript(const std::string& script, long long activityId, int quantity);
};

} // namespace seckill
