/**
 * @file OrderRepository.h
 * @brief 订单数据访问层
 *
 * 为什么这样设计？
 * - 封装 MySQL 订单表操作
 * - 提供 CRUD 基本操作
 * - 支持订单状态查询
 */

#pragma once

#include <memory>
#include <vector>
#include "model/Order.h"

namespace seckill
{

class OrderRepository
{
public:
    using Ptr = std::shared_ptr<OrderRepository>;

    OrderRepository();
    ~OrderRepository();

    // 创建订单
    bool create(const Order& order);

    // 根据ID查询订单
    Order::Ptr findById(long long id);

    // 根据订单号查询
    Order::Ptr findByOrderNo(const std::string& orderNo);

    // 查询用户的所有订单
    std::vector<Order::Ptr> findByUserId(long long userId);

    // 查询活动的所有订单
    std::vector<Order::Ptr> findByActivityId(long long activityId);

    // 更新订单状态
    bool updateStatus(long long id, int status);

    // 查询所有订单
    std::vector<Order::Ptr> findAll();

private:
    // Pimpl 模式，隐藏 MySQL 依赖
    struct Impl;
    Impl* pImpl_;
};

} // namespace seckill
