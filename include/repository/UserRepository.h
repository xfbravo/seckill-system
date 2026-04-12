/**
 * @file UserRepository.h
 * @brief 用户数据访问层
 */

#pragma once

#include <memory>
#include "model/User.h"

namespace seckill
{

class UserRepository
{
public:
    using Ptr = std::shared_ptr<UserRepository>;

    UserRepository();
    ~UserRepository();

    // 根据ID查询用户
    User::Ptr findById(long long id);

    // 创建用户（如果不存在）
    bool createIfNotExists(long long id);

    // 查询所有用户
    std::vector<User::Ptr> findAll();

private:
    struct Impl;
    Impl* pImpl_;
};

} // namespace seckill
