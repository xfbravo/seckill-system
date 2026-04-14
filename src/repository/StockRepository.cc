/**
 * @file StockRepository.cc
 * @brief 库存数据访问层实现 - 使用 Redis 连接池
 */

#include "repository/StockRepository.h"
#include "common/RedisKeys.h"
#include "common/Config.h"
#include <iostream>

namespace seckill
{

// Lua 脚本：原子扣减库存
const char* StockRepository::DECREASE_STOCK_SCRIPT = R"(
local stock = redis.call('GET', KEYS[1])
if stock == false then
    return -2
end
if tonumber(stock) >= tonumber(ARGV[1]) then
    return redis.call('DECRBY', KEYS[1], ARGV[1])
else
    return -1
end
)";

// Lua 脚本：原子秒杀操作
const char* SECKILL_SCRIPT = R"(
if redis.call('EXISTS', KEYS[2]) == 1 then
    return -2
end
local stock = redis.call('GET', KEYS[1])
if stock == false then
    return -3
end
if tonumber(stock) < tonumber(ARGV[1]) then
    return -1
end
local new_stock = redis.call('DECRBY', KEYS[1], ARGV[1])
local order_json = ARGV[2]
local json_with_stock = string.sub(order_json, 1, -2) .. ',"remain_stock":' .. new_stock .. '}'
redis.call('LPUSH', KEYS[3], json_with_stock)
redis.call('SETEX', KEYS[2], ARGV[3], '1')
return new_stock
)";

StockRepository::StockRepository()
{
    Config& cfg = Config::instance();
    redisPool_ = std::make_shared<RedisPool>(cfg.redisHost(), cfg.redisPort(), 10);
}

StockRepository::~StockRepository() = default;

bool StockRepository::initStock(long long activityId, int stock)
{
    redisContext* conn = redisPool_->get();
    if (!conn) return false;

    std::string key = RedisKeys::stockKey(activityId);
    redisReply* reply = (redisReply*)redisCommand(conn, "SET %s %d", key.c_str(), stock);

    redisPool_->release(conn);
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
    redisContext* conn = redisPool_->get();
    if (!conn) return -1;

    std::string key = RedisKeys::stockKey(activityId);
    redisReply* reply = (redisReply*)redisCommand(conn, "GET %s", key.c_str());

    redisPool_->release(conn);
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
    redisContext* conn = redisPool_->get();
    if (!conn) return false;

    std::string key = RedisKeys::stockKey(activityId);
    redisReply* reply = (redisReply*)redisCommand(conn, "INCRBY %s %d", key.c_str(), quantity);

    redisPool_->release(conn);
    if (!reply) return false;
    freeReplyObject(reply);
    return true;
}

int StockRepository::executeScript(const std::string& script, long long activityId, int quantity)
{
    redisContext* conn = redisPool_->get();
    if (!conn) return -1;

    std::string key = RedisKeys::stockKey(activityId);
    redisReply* reply = (redisReply*)redisCommand(
        conn, "EVAL %s 1 %s %d",
        script.c_str(), key.c_str(), quantity);

    redisPool_->release(conn);
    if (!reply) return -1;

    int result = -1;
    if (reply->type == REDIS_REPLY_INTEGER) {
        result = (int)reply->integer;
    }
    freeReplyObject(reply);
    return result;
}

int StockRepository::executeSeckill(long long activityId, long long userId, int quantity,
                                    const std::string& orderJson, int buyExpireSec)
{
    return executeSeckillScript(SECKILL_SCRIPT, activityId, userId, quantity, orderJson, buyExpireSec);
}

int StockRepository::executeSeckillScript(const std::string& script, long long activityId,
                                        long long userId, int quantity, const std::string& orderJson,
                                        int buyExpireSec)
{
    redisContext* conn = redisPool_->get();
    if (!conn) return -3;

    std::string stockKey = RedisKeys::stockKey(activityId);
    std::string userBuyKey = RedisKeys::userBuyKey(activityId, userId);
    std::string orderQueueKey = RedisKeys::ordersPendingKey();

    redisReply* reply = (redisReply*)redisCommand(
        conn,
        "EVAL %s 3 %s %s %s %d %s %d",
        script.c_str(),
        stockKey.c_str(),
        userBuyKey.c_str(),
        orderQueueKey.c_str(),
        quantity,
        orderJson.c_str(),
        buyExpireSec
    );

    redisPool_->release(conn);
    if (!reply) return -3;

    int result = -3;
    if (reply->type == REDIS_REPLY_INTEGER) {
        result = (int)reply->integer;
    }
    freeReplyObject(reply);
    return result;
}

} // namespace seckill