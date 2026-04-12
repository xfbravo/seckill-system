/**
 * @file User.h
 * @brief 用户模型
 */

#pragma once

#include <string>
#include <json/json.h>

namespace seckill
{

class User
{
public:
    using Ptr = std::shared_ptr<User>;

    User() = default;

    long long getId() const { return id_; }
    void setId(long long id) { id_ = id; }

    const std::string& getUsername() const { return username_; }
    void setUsername(const std::string& username) { username_ = username; }

    const std::string& getCreatedAt() const { return createdAt_; }
    void setCreatedAt(const std::string& createdAt) { createdAt_ = createdAt; }

    // 转为 JSON
    Json::Value toJson() const
    {
        Json::Value json;
        json["id"] = (int64_t)id_;
        json["username"] = username_;
        json["created_at"] = createdAt_;
        return json;
    }

private:
    long long id_ = 0;
    std::string username_;
    std::string createdAt_;
};

} // namespace seckill
