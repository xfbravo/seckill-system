/**
 * @file StockRepository.cc
 * @brief 库存数据访问层实现
 *
 * 为什么使用 Redis + Lua？
 * - Redis 是单线程执行命令，保证原子性
 * - Lua 脚本可以包含多条 Redis 命令
 * - 整个脚本执行是原子的，不会被中断
 */

#include "repository/StockRepository.h"
#include "common/RedisKeys.h"
#include "common/Config.h"
#include <iostream>
#include <hiredis/hiredis.h>
#include <mutex>

namespace seckill
{

// Lua 脚本：原子扣减库存
// 返回值：
//   >= 0: 扣减后的库存
//   -1: 库存不足
const char* StockRepository::DECREASE_STOCK_SCRIPT = R"(
local stock = redis.call('GET', KEYS[1])
if stock == false then
    return -2  -- 库存不存在
end
if tonumber(stock) >= tonumber(ARGV[1]) then
    return redis.call('DECRBY', KEYS[1], ARGV[1])
else
    return -1  -- 库存不足
end
)";

StockRepository::StockRepository()
{
    // 从配置读取 Redis 连接信息
    Config& cfg = Config::instance();

    // 连接 Redis
    redis_ = redisConnect(cfg.redisHost().c_str(), cfg.redisPort());
    if (redis_ == nullptr || redis_->err) {
        if (redis_) {
            std::cerr << "Redis connection error: " << redis_->errstr << std::endl;
            redisFree(redis_);
        }
        redis_ = nullptr;
    }
}

StockRepository::~StockRepository()
{
    if (redis_) {
        redisFree(redis_);
        redis_ = nullptr;
    }
}

bool StockRepository::initStock(long long activityId, int stock)
{
    if (!redis_) return false;

    std::string key = RedisKeys::stockKey(activityId);

    redisReply* reply = nullptr;
    {
        std::lock_guard<std::mutex> lock(redisMutex_);
        reply = (redisReply*)redisCommand(redis_, "SET %s %d", key.c_str(), stock);
    }
    if (!reply) return false;

    freeReplyObject(reply);
    return true;
}

int StockRepository::decreaseStock(long long activityId, int quantity)
{
    return executeScript(DECREASE_STOCK_SCRIPT, activityId, quantity);
}

int StockRepository::getStock(long long activityId)
{
    if (!redis_) return -1;

    std::string key = RedisKeys::stockKey(activityId);

    redisReply* reply = nullptr;
    {
        std::lock_guard<std::mutex> lock(redisMutex_);
        reply = (redisReply*)redisCommand(redis_, "GET %s", key.c_str());
    }
    if (!reply) return -1;

    if (reply->type == REDIS_REPLY_NIL) {
        freeReplyObject(reply);
        return -1;
    }

    int stock = atoi(reply->str);
    freeReplyObject(reply);
    return stock;
}

bool StockRepository::increaseStock(long long activityId, int quantity)
{
    if (!redis_) return false;

    std::string key = RedisKeys::stockKey(activityId);

    redisReply* reply = nullptr;
    {
        std::lock_guard<std::mutex> lock(redisMutex_);
        reply = (redisReply*)redisCommand(redis_, "INCRBY %s %d", key.c_str(), quantity);
    }
    if (!reply) return false;

    freeReplyObject(reply);
    return true;
}

int StockRepository::executeScript(const std::string& script, long long activityId, int quantity)
{
    if (!redis_) return -1;

    std::string key = RedisKeys::stockKey(activityId);

    redisReply* reply = nullptr;
    {
        std::lock_guard<std::mutex> lock(redisMutex_);
        reply = (redisReply*)redisCommand(
            redis_,
            "EVAL %s 1 %s %d",
            script.c_str(),
            key.c_str(),
            quantity
        );
    }

    if (!reply) return -1;

    int result = -1;
    if (reply->type == REDIS_REPLY_INTEGER) {
        result = (int)reply->integer;
    }

    freeReplyObject(reply);
    return result;
}

} // namespace seckill
