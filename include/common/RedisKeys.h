/**
 * @file RedisKeys.h
 * @brief Redis key 命名规范
 *
 * 为什么这样设计？
 * - 统一 key 命名规范，避免 key 冲突
 * - 便于查找和管理
 * - 命名格式：前缀:实体:标识
 */

#pragma once

#include <string>

namespace seckill
{

class RedisKeys
{
public:
    // Key 前缀
    static const char* PREFIX;

    // 活动相关 Key
    // 格式: activity:{id}
    // 示例: activity:1, activity:2
    static std::string activityKey(long long activityId)
    {
        return "activity:" + std::to_string(activityId);
    }

    // 库存 Key
    // 格式: stock:{activity_id}
    // 示例: stock:1, stock:2
    static std::string stockKey(long long activityId)
    {
        return "stock:" + std::to_string(activityId);
    }

    // 订单队列 Key (待处理)
    // 格式: orders:pending
    static std::string ordersPendingKey() { return "orders:pending"; }

    // 订单队列 Key (已确认)
    // 格式: orders:confirmed
    static std::string ordersConfirmedKey() { return "orders:confirmed"; }

    // 用户购买记录 Key (用于防重复购买)
    // 格式: user:buy:{activity_id}:{user_id}
    // 示例: user:buy:1:1001
    static std::string userBuyKey(long long activityId, long long userId)
    {
        return "user:buy:" + std::to_string(activityId) + ":" + std::to_string(userId);
    }
};

} // namespace seckill
