/**
 * @file Activity.h
 * @brief 秒杀活动数据模型
 *
 * 为什么这样设计？
 * - 对应数据库 seckill_activity 表
 * - 包含活动的所有属性
 * - 提供 JSON 序列化能力
 */

#pragma once

#include <string>
#include <ctime>
#include <json/json.h>

namespace seckill
{

class Activity
{
public:
    using Ptr = std::shared_ptr<Activity>;

    Activity() = default;

    // 从 JSON 构造
    Activity(const Json::Value& json)
    {
        fromJson(json);
    }

    // Getter/Setter
    long long getId() const { return id_; }
    void setId(long long id) { id_ = id; }

    const std::string& getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

    int getTotalStock() const { return totalStock_; }
    void setTotalStock(int stock) { totalStock_ = stock; }

    int getRemainStock() const { return remainStock_; }
    void setRemainStock(int stock) { remainStock_ = stock; }

    double getPrice() const { return price_; }
    void setPrice(double price) { price_ = price; }

    const std::string& getStartTime() const { return startTime_; }
    void setStartTime(const std::string& time) { startTime_ = time; }

    const std::string& getEndTime() const { return endTime_; }
    void setEndTime(const std::string& time) { endTime_ = time; }

    int getStatus() const { return status_; }
    void setStatus(int status) { status_ = status; }

    const std::string& getCreatedAt() const { return createdAt_; }
    void setCreatedAt(const std::string& time) { createdAt_ = time; }

    // 状态枚举
    enum Status
    {
        NOT_START = 0,  // 未开始
        ACTIVE = 1,     // 进行中
        ENDED = 2       // 已结束
    };

    // 转为 JSON
    Json::Value toJson() const
    {
        Json::Value json;
        json["id"] = (int64_t)id_;
        json["name"] = name_;
        json["total_stock"] = totalStock_;
        json["remain_stock"] = remainStock_;
        json["price"] = price_;
        json["start_time"] = startTime_;
        json["end_time"] = endTime_;
        json["status"] = status_;
        json["created_at"] = createdAt_;
        return json;
    }

    // 从 JSON 解析
    void fromJson(const Json::Value& json)
    {
        if (json.isMember("id")) id_ = json["id"].asInt64();
        if (json.isMember("name")) name_ = json["name"].asString();
        if (json.isMember("total_stock")) totalStock_ = json["total_stock"].asInt();
        if (json.isMember("remain_stock")) remainStock_ = json["remain_stock"].asInt();
        if (json.isMember("price")) price_ = json["price"].asDouble();
        if (json.isMember("start_time")) startTime_ = json["start_time"].asString();
        if (json.isMember("end_time")) endTime_ = json["end_time"].asString();
        if (json.isMember("status")) status_ = json["status"].asInt();
        if (json.isMember("created_at")) createdAt_ = json["created_at"].asString();
    }

private:
    long long id_ = 0;          // 活动ID
    std::string name_;          // 活动名称
    int totalStock_ = 0;        // 总库存
    int remainStock_ = 0;       // 剩余库存
    double price_ = 0;          // 秒杀价格
    std::string startTime_;     // 开始时间
    std::string endTime_;       // 结束时间
    int status_ = 0;            // 状态
    std::string createdAt_;     // 创建时间
};

} // namespace seckill
