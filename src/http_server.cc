/**
 * @file http_server.cc
 * @brief HTTP 服务器实现 - 使用 cpp-httplib
 */

#include "http_server.h"
#include "service/ActivityService.h"
#include "service/SeckillService.h"
#include "service/OrderService.h"
#include <iostream>
#include <json/json.h>

using namespace httplib;

namespace seckill
{

HttpServer::HttpServer(int port) : port_(port)
{
}

void HttpServer::start()
{
    svr_ = std::make_shared<httplib::Server>();
    setupRoutes();

    std::cout << "HTTP Server starting on port " << port_ << "..." << std::endl;
    svr_->listen("0.0.0.0", port_);
}

void HttpServer::setupRoutes()
{
    // 活动管理接口
    svr_->Post("/api/activity/create", [this](const Request& req, Response& res) {
        handleCreateActivity(req, res);
    });

    svr_->Get("/api/activity/list", [this](const Request& req, Response& res) {
        handleListActivity(req, res);
    });

    svr_->Get(R"(/api/activity/(\d+))", [this](const Request& req, Response& res) {
        handleGetActivity(req, res);
    });

    svr_->Get(R"(/api/activity/(\d+)/stock)", [this](const Request& req, Response& res) {
        handleGetStock(req, res);
    });

    // 秒杀接口
    svr_->Post("/api/seckill/order", [this](const Request& req, Response& res) {
        handleSeckillOrder(req, res);
    });

    svr_->Get(R"(/api/seckill/countdown/(\d+))", [this](const Request& req, Response& res) {
        handleGetCountdown(req, res);
    });

    // 订单接口
    svr_->Get(R"(/api/order/(\d+))", [this](const Request& req, Response& res) {
        handleGetOrder(req, res);
    });

    svr_->Get("/api/order/list", [this](const Request& req, Response& res) {
        handleListOrders(req, res);
    });

    svr_->Get(R"(/api/order/status/(\d+))", [this](const Request& req, Response& res) {
        handleGetOrderStatus(req, res);
    });

    std::cout << "Routes registered successfully" << std::endl;
}

void HttpServer::handleCreateActivity(const Request& req, Response& res)
{
    Json::Value json;

    try {
        std::string body = req.body;
        if (body.empty()) {
            json["code"] = -1;
            json["message"] = "Empty request body";
            res.set_content(json.toStyledString(), "application/json");
            return;
        }

        Json::Value requestJson;
        std::istringstream stream(body);
        Json::CharReaderBuilder builder;
        std::string errs;
        if (!Json::parseFromStream(builder, stream, &requestJson, &errs)) {
            json["code"] = -1;
            json["message"] = "Invalid JSON";
            res.set_content(json.toStyledString(), "application/json");
            return;
        }

        if (!requestJson.isMember("name") || !requestJson.isMember("total_stock") ||
            !requestJson.isMember("price") || !requestJson.isMember("start_time") ||
            !requestJson.isMember("end_time")) {
            json["code"] = -1;
            json["message"] = "Missing required parameters";
            res.set_content(json.toStyledString(), "application/json");
            return;
        }

        static ActivityService::Ptr service = std::make_shared<ActivityService>();
        auto result = service->createActivity(
            requestJson["name"].asString(),
            requestJson["total_stock"].asInt(),
            requestJson["price"].asDouble(),
            requestJson["start_time"].asString(),
            requestJson["end_time"].asString()
        );

        json["code"] = result->code();
        json["message"] = result->message();
        json["data"] = result->data();

    } catch (const std::exception& e) {
        json["code"] = -1;
        json["message"] = e.what();
    }

    res.set_content(json.toStyledString(), "application/json");
}

void HttpServer::handleListActivity(const Request& req, Response& res)
{
    Json::Value json;

    try {
        static ActivityService::Ptr service = std::make_shared<ActivityService>();
        auto result = service->getActivityList();

        json["code"] = result->code();
        json["message"] = result->message();
        json["data"] = result->data();

    } catch (const std::exception& e) {
        json["code"] = -1;
        json["message"] = e.what();
    }

    res.set_content(json.toStyledString(), "application/json");
}

void HttpServer::handleGetActivity(const Request& req, Response& res)
{
    Json::Value json;
    long long id = std::stoll(req.matches[1]);

    try {
        static ActivityService::Ptr service = std::make_shared<ActivityService>();
        auto result = service->getActivity(id);

        json["code"] = result->code();
        json["message"] = result->message();
        json["data"] = result->data();

    } catch (const std::exception& e) {
        json["code"] = -1;
        json["message"] = e.what();
    }

    res.set_content(json.toStyledString(), "application/json");
}

void HttpServer::handleGetStock(const Request& req, Response& res)
{
    Json::Value json;
    long long id = std::stoll(req.matches[1]);

    try {
        static ActivityService::Ptr service = std::make_shared<ActivityService>();
        auto result = service->getActivityStock(id);

        json["code"] = result->code();
        json["message"] = result->message();
        json["data"] = result->data();

    } catch (const std::exception& e) {
        json["code"] = -1;
        json["message"] = e.what();
    }

    res.set_content(json.toStyledString(), "application/json");
}

void HttpServer::handleSeckillOrder(const Request& req, Response& res)
{
    Json::Value json;

    try {
        std::string body = req.body;
        if (body.empty()) {
            json["code"] = -1;
            json["message"] = "Empty request body";
            res.set_content(json.toStyledString(), "application/json");
            return;
        }

        Json::Value requestJson;
        std::istringstream stream(body);
        Json::CharReaderBuilder builder;
        std::string errs;
        if (!Json::parseFromStream(builder, stream, &requestJson, &errs)) {
            json["code"] = -1;
            json["message"] = "Invalid JSON";
            res.set_content(json.toStyledString(), "application/json");
            return;
        }

        if (!requestJson.isMember("activity_id") || !requestJson.isMember("user_id")) {
            json["code"] = -1;
            json["message"] = "Missing required parameters: activity_id, user_id";
            res.set_content(json.toStyledString(), "application/json");
            return;
        }

        long long activityId = requestJson["activity_id"].asInt64();
        long long userId = requestJson["user_id"].asInt64();
        int quantity = requestJson.isMember("quantity") ? requestJson["quantity"].asInt() : 1;

        static SeckillService::Ptr service = std::make_shared<SeckillService>();
        auto result = service->executeSeckill(activityId, userId, quantity);

        json["code"] = result->code();
        json["message"] = result->message();
        json["data"] = result->data();

    } catch (const std::exception& e) {
        json["code"] = -1;
        json["message"] = e.what();
    }

    res.set_content(json.toStyledString(), "application/json");
}

void HttpServer::handleGetCountdown(const Request& req, Response& res)
{
    Json::Value json;
    long long id = std::stoll(req.matches[1]);

    try {
        static ActivityService::Ptr service = std::make_shared<ActivityService>();
        auto result = service->getCountdown(id);

        json["code"] = result->code();
        json["message"] = result->message();
        json["data"] = result->data();

    } catch (const std::exception& e) {
        json["code"] = -1;
        json["message"] = e.what();
    }

    res.set_content(json.toStyledString(), "application/json");
}

void HttpServer::handleGetOrder(const Request& req, Response& res)
{
    Json::Value json;
    long long id = std::stoll(req.matches[1]);

    try {
        static OrderService::Ptr service = std::make_shared<OrderService>();
        auto result = service->getOrderById(id);

        json["code"] = result->code();
        json["message"] = result->message();
        json["data"] = result->data();

    } catch (const std::exception& e) {
        json["code"] = -1;
        json["message"] = e.what();
    }

    res.set_content(json.toStyledString(), "application/json");
}

void HttpServer::handleListOrders(const Request& req, Response& res)
{
    Json::Value json;

    try {
        static OrderService::Ptr service = std::make_shared<OrderService>();
        auto result = service->getAllOrders();

        json["code"] = result->code();
        json["message"] = result->message();
        json["data"] = result->data();

    } catch (const std::exception& e) {
        json["code"] = -1;
        json["message"] = e.what();
    }

    res.set_content(json.toStyledString(), "application/json");
}

void HttpServer::handleGetOrderStatus(const Request& req, Response& res)
{
    Json::Value json;
    long long id = std::stoll(req.matches[1]);

    try {
        static OrderService::Ptr service = std::make_shared<OrderService>();
        auto result = service->getOrderById(id);

        if (result->code() == 0) {
            json["code"] = 0;
            json["message"] = result->message();
            json["data"]["id"] = (int64_t)id;
            json["data"]["status"] = result->data()["status"];
            json["data"]["status_text"] = service->getStatusText(result->data()["status"].asInt());
        } else {
            json["code"] = result->code();
            json["message"] = result->message();
        }

    } catch (const std::exception& e) {
        json["code"] = -1;
        json["message"] = e.what();
    }

    res.set_content(json.toStyledString(), "application/json");
}

} // namespace seckill
