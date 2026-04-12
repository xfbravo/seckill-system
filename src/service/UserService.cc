/**
 * @file UserService.cc
 * @brief 用户服务层实现
 */

#include "service/UserService.h"
#include "repository/UserRepository.h"
#include "common/ErrorCode.h"
#include <iostream>

namespace seckill
{

struct UserService::Impl
{
    UserRepository::Ptr userRepo;
};

UserService::UserService()
{
    pImpl_ = new Impl();
    pImpl_->userRepo = std::make_shared<UserRepository>();
}

UserService::~UserService()
{
    delete pImpl_;
}

Result::Ptr UserService::loginOrCreate(long long userId)
{
    try {
        // 尝试创建用户（如果不存在）
        if (!pImpl_->userRepo->createIfNotExists(userId)) {
            return Result::fail(ErrorCode::ERR_INTERNAL, "Failed to create user");
        }

        // 获取用户信息
        auto user = pImpl_->userRepo->findById(userId);
        if (!user) {
            return Result::fail(ErrorCode::ERR_USER_NOT_FOUND, "User not found");
        }

        return Result::success(user->toJson());

    } catch (const std::exception& e) {
        std::cerr << "loginOrCreate exception: " << e.what() << std::endl;
        return Result::fail(ErrorCode::ERR_INTERNAL, e.what());
    }
}

Result::Ptr UserService::getUser(long long userId)
{
    try {
        auto user = pImpl_->userRepo->findById(userId);
        if (!user) {
            return Result::fail(ErrorCode::ERR_USER_NOT_FOUND, "User not found");
        }

        return Result::success(user->toJson());

    } catch (const std::exception& e) {
        std::cerr << "getUser exception: " << e.what() << std::endl;
        return Result::fail(ErrorCode::ERR_INTERNAL, e.what());
    }
}

Result::Ptr UserService::getAllUsers()
{
    try {
        auto users = pImpl_->userRepo->findAll();

        Json::Value data;
        for (const auto& user : users) {
            data.append(user->toJson());
        }

        return Result::success(data);

    } catch (const std::exception& e) {
        std::cerr << "getAllUsers exception: " << e.what() << std::endl;
        return Result::fail(ErrorCode::ERR_INTERNAL, e.what());
    }
}

} // namespace seckill
