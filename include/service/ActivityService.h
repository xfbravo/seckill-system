/**
 * @file ActivityService.h
 * @brief 活动服务层
 */

#pragma once

#include <memory>
#include <vector>
#include "common/Result.h"
#include "model/Activity.h"

namespace seckill
{

class ActivityService
{
public:
    using Ptr = std::shared_ptr<ActivityService>;

    ActivityService();
    ~ActivityService();

    Result::Ptr createActivity(
        const std::string& name,
        int totalStock,
        double price,
        const std::string& startTime,
        const std::string& endTime
    );

    Result::Ptr getActivityList();
    Result::Ptr getActivity(long long id);
    Result::Ptr getActivityStock(long long id);
    Result::Ptr checkActivityStatus(long long id);
    Result::Ptr getCountdown(long long id);

private:
    struct Impl;
    Impl* pImpl_;
};

} // namespace seckill
