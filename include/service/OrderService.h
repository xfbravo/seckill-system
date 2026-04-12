/**
 * @file OrderService.h
 * @brief 订单服务层
 */

#pragma once

#include <memory>
#include <vector>
#include "common/Result.h"
#include "model/Order.h"

namespace seckill
{

class OrderService
{
public:
    using Ptr = std::shared_ptr<OrderService>;

    OrderService();
    ~OrderService();

    Result::Ptr getOrderByNo(const std::string& orderNo);
    Result::Ptr getOrderById(long long id);
    Result::Ptr getOrdersByUserId(long long userId);
    Result::Ptr getAllOrders();
    Result::Ptr updateOrderStatus(long long id, int status);
    std::string getStatusText(int status);

private:
    struct Impl;
    Impl* pImpl_;
};

} // namespace seckill
