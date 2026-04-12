/**
 * @file OrderController.cc
 * @brief 订单查询接口实现
 */

#include "controller/OrderController.h"
#include "service/OrderService.h"
#include <drogon/HttpResponse.h>
#include <drogon/HttpRequest.h>
#include <iostream>
#include <json/json.h>

namespace seckill
{

static drogon::HttpResponsePtr makeJsonResponse(int code, const std::string& message, const Json::Value& data)
{
    Json::Value json;
    json["code"] = code;
    json["message"] = message;
    json["data"] = data;

    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(drogon::k200OK);
    resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    resp->setBody(json.toStyledString());
    return resp;
}

static drogon::HttpResponsePtr makeErrorResponse(int code, const std::string& message)
{
    return makeJsonResponse(code, message, Json::Value::nullSingleton);
}

void OrderController::detail(const drogon::HttpRequestPtr& req,
                              std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                              long long id)
{
    static OrderService::Ptr service = std::make_shared<OrderService>();
    auto result = service->getOrderById(id);

    if (result->code() == 0) {
        callback(makeJsonResponse(0, result->message(), result->data()));
    } else {
        callback(makeErrorResponse(result->code(), result->message()));
    }
}

void OrderController::list(const drogon::HttpRequestPtr& req,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    static OrderService::Ptr service = std::make_shared<OrderService>();
    auto result = service->getAllOrders();

    if (result->code() == 0) {
        callback(makeJsonResponse(0, result->message(), result->data()));
    } else {
        callback(makeErrorResponse(result->code(), result->message()));
    }
}

void OrderController::status(const drogon::HttpRequestPtr& req,
                              std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                              long long id)
{
    static OrderService::Ptr service = std::make_shared<OrderService>();
    auto result = service->getOrderById(id);

    if (result->code() == 0) {
        Json::Value data;
        data["id"] = (int64_t)id;
        data["status"] = result->data()["status"];
        data["status_text"] = service->getStatusText(result->data()["status"].asInt());
        callback(makeJsonResponse(0, result->message(), data));
    } else {
        callback(makeErrorResponse(result->code(), result->message()));
    }
}

} // namespace seckill
