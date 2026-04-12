/**
 * @file ActivityController.cc
 * @brief 活动管理接口实现
 */

#include "controller/ActivityController.h"
#include "service/ActivityService.h"
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

void ActivityController::create(const drogon::HttpRequestPtr& req,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto jsonPtr = req->getJsonObject();
    if (!jsonPtr) {
        callback(makeErrorResponse(-1, "Invalid request body"));
        return;
    }

    const Json::Value& json = *jsonPtr;

    if (!json.isMember("name") || !json.isMember("total_stock") ||
        !json.isMember("price") || !json.isMember("start_time") ||
        !json.isMember("end_time")) {
        callback(makeErrorResponse(-1, "Missing required parameters"));
        return;
    }

    std::string name = json["name"].asString();
    int totalStock = json["total_stock"].asInt();
    double price = json["price"].asDouble();
    std::string startTime = json["start_time"].asString();
    std::string endTime = json["end_time"].asString();

    static ActivityService::Ptr service = std::make_shared<ActivityService>();
    auto result = service->createActivity(name, totalStock, price, startTime, endTime);

    if (result->code() == 0) {
        callback(makeJsonResponse(0, result->message(), result->data()));
    } else {
        callback(makeErrorResponse(result->code(), result->message()));
    }
}

void ActivityController::list(const drogon::HttpRequestPtr& req,
                               std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    static ActivityService::Ptr service = std::make_shared<ActivityService>();
    auto result = service->getActivityList();

    if (result->code() == 0) {
        callback(makeJsonResponse(0, result->message(), result->data()));
    } else {
        callback(makeErrorResponse(result->code(), result->message()));
    }
}

void ActivityController::detail(const drogon::HttpRequestPtr& req,
                                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                long long id)
{
    static ActivityService::Ptr service = std::make_shared<ActivityService>();
    auto result = service->getActivity(id);

    if (result->code() == 0) {
        callback(makeJsonResponse(0, result->message(), result->data()));
    } else {
        callback(makeErrorResponse(result->code(), result->message()));
    }
}

void ActivityController::stock(const drogon::HttpRequestPtr& req,
                                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                long long id)
{
    static ActivityService::Ptr service = std::make_shared<ActivityService>();
    auto result = service->getActivityStock(id);

    if (result->code() == 0) {
        callback(makeJsonResponse(0, result->message(), result->data()));
    } else {
        callback(makeErrorResponse(result->code(), result->message()));
    }
}

} // namespace seckill
