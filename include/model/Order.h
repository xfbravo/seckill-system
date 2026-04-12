/**
 * @file Order.h
 * @brief 订单数据模型
 *
 * 为什么这样设计？
 * - 对应数据库 seckill_order 表
 * - 包含订单的所有属性
 * - 提供 JSON 序列化能力
 */

#pragma once

#include <string>
#include <json/json.h>

namespace seckill
{

class Order
{
public:
    using Ptr = std::shared_ptr<Order>;

    Order() = default;

    // 从 JSON 构造
    Order(const Json::Value& json)
    {
        fromJson(json);
    }

    // Getter/Setter
    long long getId() const { return id_; }
    void setId(long long id) { id_ = id; }

    const std::string& getOrderNo() const { return orderNo_; }
    void setOrderNo(const std::string& no) { orderNo_ = no; }

    long long getActivityId() const { return activityId_; }
    void setActivityId(long long id) { activityId_ = id; }

    long long getUserId() const { return userId_; }
    void setUserId(long long id) { userId_ = id; }

    int getQuantity() const { return quantity_; }
    void setQuantity(int qty) { quantity_ = qty; }

    int getStatus() const { return status_; }
    void setStatus(int status) { status_ = status; }

    const std::string& getCreatedAt() const { return createdAt_; }
    void setCreatedAt(const std::string& time) { createdAt_ = time; }

    // 状态枚举
    enum Status
    {
        PENDING = 0,     // 待确认
        CONFIRMED = 1,   // 已确认
        CANCELLED = 2    // 已取消
    };

    // 转为 JSON
    Json::Value toJson() const
    {
        Json::Value json;
        json["id"] = (int64_t)id_;
        json["order_no"] = orderNo_;
        json["activity_id"] = (int64_t)activityId_;
        json["user_id"] = (int64_t)userId_;
        json["quantity"] = quantity_;
        json["status"] = status_;
        json["created_at"] = createdAt_;
        return json;
    }

    // 从 JSON 解析
    void fromJson(const Json::Value& json)
    {
        if (json.isMember("id")) id_ = json["id"].asInt64();
        if (json.isMember("order_no")) orderNo_ = json["order_no"].asString();
        if (json.isMember("activity_id")) activityId_ = json["activity_id"].asInt64();
        if (json.isMember("user_id")) userId_ = json["user_id"].asInt64();
        if (json.isMember("quantity")) quantity_ = json["quantity"].asInt();
        if (json.isMember("status")) status_ = json["status"].asInt();
        if (json.isMember("created_at")) createdAt_ = json["created_at"].asString();
    }

private:
    long long id_ = 0;          // 订单ID
    std::string orderNo_;       // 订单号
    long long activityId_ = 0;  // 活动ID
    long long userId_ = 0;      // 用户ID
    int quantity_ = 1;           // 购买数量
    int status_ = 0;            // 状态
    std::string createdAt_;      // 创建时间
};

} // namespace seckill
