/**
 * @file ActivityController.h
 * @brief 活动管理接口
 *
 * 为什么这样设计？
 * - Drogon 控制器，处理 HTTP 请求
 * - 提供活动创建、查询等 RESTful API
 * - 负责参数校验和返回格式
 */

#pragma once

#include <drogon/HttpController.h>

namespace seckill
{

class ActivityController : public drogon::HttpController<ActivityController>
{
public:
    // 路由配置
    METHOD_LIST_BEGIN
    // POST /api/activity/create - 创建活动
    METHOD_ADD(ActivityController::create, "/api/activity/create", drogon::Post);

    // GET /api/activity/list - 获取活动列表
    METHOD_ADD(ActivityController::list, "/api/activity/list", drogon::Get);

    // GET /api/activity/{id} - 获取活动详情
    METHOD_ADD(ActivityController::detail, "/api/activity/{id}", drogon::Get);

    // GET /api/activity/{id}/stock - 查询库存
    METHOD_ADD(ActivityController::stock, "/api/activity/{id}/stock", drogon::Get);
    METHOD_LIST_END

    // 创建秒杀活动
    // 请求体: { "name": "活动名称", "total_stock": 100, "price": 99.99, "start_time": "2026-04-15 10:00:00", "end_time": "2026-04-15 12:00:00" }
    void create(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    // 获取活动列表
    void list(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    // 获取活动详情
    void detail(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, long long id);

    // 查询活动库存
    void stock(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, long long id);
};

} // namespace seckill
