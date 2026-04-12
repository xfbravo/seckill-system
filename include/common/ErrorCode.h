/**
 * @file ErrorCode.h
 * @brief 错误码定义
 *
 * 为什么这样设计？
 * - 错误码集中管理，便于维护和定位问题
 * - 客户端可以根据错误码做不同处理
 * - 数字从 1 开始，0 表示成功
 */

#pragma once

#include <string>
#include <unordered_map>

namespace seckill
{

class ErrorCode
{
public:
    // 错误码枚举
    enum Code
    {
        SUCCESS = 0,

        // 通用错误 (1xxx)
        ERR_INTERNAL = 1001,      // 内部错误
        ERR_PARAM_INVALID = 1002,  // 参数无效
        ERR_NOT_FOUND = 1003,     // 资源不存在

        // 活动相关错误 (2xxx)
        ERR_ACTIVITY_NOT_FOUND = 2001,   // 活动不存在
        ERR_ACTIVITY_NOT_START = 2002,   // 活动未开始
        ERR_ACTIVITY_ENDED = 2003,       // 活动已结束
        ERR_ACTIVITY_SOLD_OUT = 2004,    // 活动已售罄

        // 订单相关错误 (3xxx)
        ERR_ORDER_NOT_FOUND = 3001,      // 订单不存在
        ERR_ORDER_CREATE_FAIL = 3002,    // 订单创建失败
        ERR_ORDER_STATUS_INVALID = 3003, // 订单状态无效

        // 库存相关错误 (4xxx)
        ERR_STOCK_NOT_ENOUGH = 4001,     // 库存不足
        ERR_STOCK_INIT_FAIL = 4002,      // 库存初始化失败
    };

    // 获取错误码对应的消息
    static std::string getMessage(int code)
    {
        static std::unordered_map<int, std::string> messages = {
            {SUCCESS, "success"},
            {ERR_INTERNAL, "Internal server error"},
            {ERR_PARAM_INVALID, "Invalid parameter"},
            {ERR_NOT_FOUND, "Resource not found"},
            {ERR_ACTIVITY_NOT_FOUND, "Activity not found"},
            {ERR_ACTIVITY_NOT_START, "Activity has not started"},
            {ERR_ACTIVITY_ENDED, "Activity has ended"},
            {ERR_ACTIVITY_SOLD_OUT, "Activity is sold out"},
            {ERR_ORDER_NOT_FOUND, "Order not found"},
            {ERR_ORDER_CREATE_FAIL, "Failed to create order"},
            {ERR_ORDER_STATUS_INVALID, "Invalid order status"},
            {ERR_STOCK_NOT_ENOUGH, "Stock not enough"},
            {ERR_STOCK_INIT_FAIL, "Failed to initialize stock"},
        };

        auto it = messages.find(code);
        if (it != messages.end()) {
            return it->second;
        }
        return "Unknown error";
    }

    // 判断是否成功
    static bool isSuccess(int code) { return code == SUCCESS; }
    static bool isFail(int code) { return code != SUCCESS; }
};

} // namespace seckill
