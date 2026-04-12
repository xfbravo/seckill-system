/**
 * @file OrderController.h
 * @brief 订单查询接口
 *
 * 为什么这样设计？
 * - 处理订单查询请求
 * - 提供订单详情、订单列表等接口
 */

#pragma once

#include <drogon/HttpController.h>

namespace seckill
{

class OrderController : public drogon::HttpController<OrderController>
{
public:
    // 路由配置
    METHOD_LIST_BEGIN
    // GET /api/order/{id} - 查询订单详情
    METHOD_ADD(OrderController::detail, "/api/order/{id}", drogon::Get);

    // GET /api/order/list - 查询订单列表
    METHOD_ADD(OrderController::list, "/api/order/list", drogon::Get);

    // GET /api/order/status/{id} - 查询订单状态
    METHOD_ADD(OrderController::status, "/api/order/status/{id}", drogon::Get);
    METHOD_LIST_END

    // 查询订单详情
    void detail(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, long long id);

    // 查询订单列表
    void list(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    // 查询订单状态
    void status(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, long long id);
};

} // namespace seckill
