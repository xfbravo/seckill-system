/**
 * @file OrderService.cc
 * @brief 订单服务层实现
 */

#include "service/OrderService.h"
#include "repository/OrderRepository.h"
#include "common/ErrorCode.h"
#include <iostream>

namespace seckill
{

struct OrderService::Impl
{
    OrderRepository::Ptr orderRepo;
};

OrderService::OrderService()
{
    pImpl_ = new Impl();
    pImpl_->orderRepo = std::make_shared<OrderRepository>();
}

OrderService::~OrderService()
{
    delete pImpl_;
}

Result::Ptr OrderService::getOrderByNo(const std::string& orderNo)
{
    try {
        auto order = pImpl_->orderRepo->findByOrderNo(orderNo);
        if (!order) {
            return Result::fail(ErrorCode::ERR_ORDER_NOT_FOUND, "Order not found");
        }

        return Result::success(order->toJson());

    } catch (const std::exception& e) {
        std::cerr << "getOrderByNo exception: " << e.what() << std::endl;
        return Result::fail(ErrorCode::ERR_INTERNAL, e.what());
    }
}

Result::Ptr OrderService::getOrderById(long long id)
{
    try {
        auto order = pImpl_->orderRepo->findById(id);
        if (!order) {
            return Result::fail(ErrorCode::ERR_ORDER_NOT_FOUND, "Order not found");
        }

        return Result::success(order->toJson());

    } catch (const std::exception& e) {
        std::cerr << "getOrderById exception: " << e.what() << std::endl;
        return Result::fail(ErrorCode::ERR_INTERNAL, e.what());
    }
}

Result::Ptr OrderService::getOrdersByUserId(long long userId)
{
    try {
        auto orders = pImpl_->orderRepo->findByUserId(userId);

        Json::Value data;
        for (const auto& order : orders) {
            data.append(order->toJson());
        }

        return Result::success(data);

    } catch (const std::exception& e) {
        std::cerr << "getOrdersByUserId exception: " << e.what() << std::endl;
        return Result::fail(ErrorCode::ERR_INTERNAL, e.what());
    }
}

Result::Ptr OrderService::getAllOrders()
{
    try {
        auto orders = pImpl_->orderRepo->findAll();

        Json::Value data;
        for (const auto& order : orders) {
            data.append(order->toJson());
        }

        return Result::success(data);

    } catch (const std::exception& e) {
        std::cerr << "getAllOrders exception: " << e.what() << std::endl;
        return Result::fail(ErrorCode::ERR_INTERNAL, e.what());
    }
}

Result::Ptr OrderService::updateOrderStatus(long long id, int status)
{
    try {
        if (pImpl_->orderRepo->updateStatus(id, status)) {
            return Result::success();
        }
        return Result::fail(ErrorCode::ERR_ORDER_STATUS_INVALID, "Failed to update order status");
    } catch (const std::exception& e) {
        std::cerr << "updateOrderStatus exception: " << e.what() << std::endl;
        return Result::fail(ErrorCode::ERR_INTERNAL, e.what());
    }
}

std::string OrderService::getStatusText(int status)
{
    switch (status) {
        case 0: return "待确认";
        case 1: return "已确认";
        case 2: return "已取消";
        default: return "未知状态";
    }
}

} // namespace seckill
