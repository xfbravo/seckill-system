/**
 * @file UserService.h
 * @brief 用户服务层
 */

#pragma once

#include <memory>
#include "common/Result.h"
#include "model/User.h"

namespace seckill
{

class UserService
{
public:
    using Ptr = std::shared_ptr<UserService>;

    UserService();
    ~UserService();

    // 登录或创建用户
    Result::Ptr loginOrCreate(long long userId);

    // 获取用户信息
    Result::Ptr getUser(long long userId);

    // 获取所有用户
    Result::Ptr getAllUsers();

private:
    struct Impl;
    Impl* pImpl_;
};

} // namespace seckill
