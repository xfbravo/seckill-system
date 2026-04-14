/**
 * @file ActivityService.h
 * @brief 活动服务层
 */

#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <shared_mutex>
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

    // 获取活动（先查缓存，再查数据库）
    Activity::Ptr getActivityFromCacheOrDb(long long id);

private:
    struct Impl;
    Impl* pImpl_;

    int computeStatus(const std::string& startTime, const std::string& endTime) const;
};

} // namespace seckill
