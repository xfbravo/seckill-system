/**
 * @file http_server.h
 * @brief HTTP 服务器 - 使用 cpp-httplib
 */

#pragma once

#include <memory>
#include <string>
#include <httplib.h>
#include "service/ActivityService.h"
#include "service/SeckillService.h"
#include "service/OrderService.h"

namespace seckill
{

class HttpServer
{
public:
    using Ptr = std::shared_ptr<HttpServer>;

    HttpServer(int port = 8080);
    ~HttpServer() = default;

    // 启动服务器
    void start();

private:
    void setupRoutes();

    // 活动接口处理
    void handleCreateActivity(const httplib::Request& req, httplib::Response& res);
    void handleListActivity(const httplib::Request& req, httplib::Response& res);
    void handleGetActivity(const httplib::Request& req, httplib::Response& res);
    void handleGetStock(const httplib::Request& req, httplib::Response& res);

    // 秒杀接口处理
    void handleSeckillOrder(const httplib::Request& req, httplib::Response& res);
    void handleGetCountdown(const httplib::Request& req, httplib::Response& res);

    // 订单接口处理
    void handleGetOrder(const httplib::Request& req, httplib::Response& res);
    void handleListOrders(const httplib::Request& req, httplib::Response& res);
    void handleGetOrderStatus(const httplib::Request& req, httplib::Response& res);

    // 用户接口处理
    void handleUserLogin(const httplib::Request& req, httplib::Response& res);

    int port_;
    std::shared_ptr<httplib::Server> svr_;
};

} // namespace seckill
