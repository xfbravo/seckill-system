/**
 * @file Result.h
 * @brief 统一返回格式
 */

#pragma once

#include <string>
#include <json/json.h>

namespace seckill
{

class Result
{
public:
    using Ptr = std::shared_ptr<Result>;

    Result() : code_(0), message_("success"), data_(Json::Value::nullSingleton) {}
    Result(int code, const std::string& message) : code_(code), message_(message), data_(Json::Value::nullSingleton) {}
    Result(int code, const std::string& message, const Json::Value& data) : code_(code), message_(message), data_(data) {}

    static Result::Ptr success() { return std::make_shared<Result>(0, "success"); }
    static Result::Ptr success(const std::string& message) { return std::make_shared<Result>(0, message); }
    static Result::Ptr success(const Json::Value& data) { return std::make_shared<Result>(0, "success", data); }
    static Result::Ptr success(const std::string& message, const Json::Value& data) { return std::make_shared<Result>(0, message, data); }

    static Result::Ptr fail(int code) { return std::make_shared<Result>(code, "error"); }
    static Result::Ptr fail(int code, const std::string& message) { return std::make_shared<Result>(code, message); }
    static Result::Ptr fail(const std::string& message) { return std::make_shared<Result>(-1, message); }

    Json::Value toJson() const
    {
        Json::Value json;
        json["code"] = code_;
        json["message"] = message_;
        json["data"] = data_;
        return json;
    }

    int code() const { return code_; }
    const std::string& message() const { return message_; }
    const Json::Value& data() const { return data_; }

private:
    int code_;
    std::string message_;
    Json::Value data_;
};

} // namespace seckill
