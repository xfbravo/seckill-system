/**
 * @file SeckillController.h
 * @brief 秒杀接口
 *
 * 为什么这样设计？
 * - 处理秒杀下单请求
 * - 接收用户秒杀请求，调用 SeckillService
 * - 返回订单号或错误信息
 */

#pragma once

#include <drogon/HttpController.h>

namespace seckill
{

class SeckillController : public drogon::HttpController<SeckillController>
{
public:
    // 路由配置
    METHOD_LIST_BEGIN
    // POST /api/seckill/order - 提交秒杀订单
    METHOD_ADD(SeckillController::order, "/api/seckill/order", drogon::Post);

    // GET /api/seckill/countdown/{id} - 获取活动倒计时
    METHOD_ADD(SeckillController::countdown, "/api/seckill/countdown/{id}", drogon::Get);
    METHOD_LIST_END

    // 提交秒杀订单
    // 请求体: { "activity_id": 1, "user_id": 1001, "quantity": 1 }
    void order(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    // 获取活动倒计时
    void countdown(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, long long id);
};

} // namespace seckill
