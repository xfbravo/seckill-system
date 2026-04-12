/**
 * @file SeckillController.cc
 * @brief 秒杀接口实现
 */

#include "controller/SeckillController.h"
#include "service/SeckillService.h"
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

void SeckillController::order(const drogon::HttpRequestPtr& req,
                              std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto jsonPtr = req->getJsonObject();
    if (!jsonPtr) {
        callback(makeErrorResponse(-1, "Invalid request body"));
        return;
    }

    const Json::Value& json = *jsonPtr;

    if (!json.isMember("activity_id") || !json.isMember("user_id")) {
        callback(makeErrorResponse(-1, "Missing required parameters: activity_id, user_id"));
        return;
    }

    long long activityId = json["activity_id"].asInt64();
    long long userId = json["user_id"].asInt64();
    int quantity = json.isMember("quantity") ? json["quantity"].asInt() : 1;

    static SeckillService::Ptr service = std::make_shared<SeckillService>();
    auto result = service->executeSeckill(activityId, userId, quantity);

    if (result->code() == 0) {
        callback(makeJsonResponse(0, result->message(), result->data()));
    } else {
        callback(makeErrorResponse(result->code(), result->message()));
    }
}

void SeckillController::countdown(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                   long long id)
{
    static ActivityService::Ptr activityService = std::make_shared<ActivityService>();
    auto result = activityService->getCountdown(id);

    if (result->code() == 0) {
        callback(makeJsonResponse(0, result->message(), result->data()));
    } else {
        callback(makeErrorResponse(result->code(), result->message()));
    }
}

} // namespace seckill
