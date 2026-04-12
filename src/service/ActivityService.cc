/**
 * @file ActivityService.cc
 * @brief 活动服务层实现
 */

#include "service/ActivityService.h"
#include "repository/ActivityRepository.h"
#include "repository/StockRepository.h"
#include "common/ErrorCode.h"
#include <iostream>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace seckill
{

struct ActivityService::Impl
{
    ActivityRepository::Ptr activityRepo;
    StockRepository::Ptr stockRepo;
};

ActivityService::ActivityService()
{
    pImpl_ = new Impl();
    pImpl_->activityRepo = std::make_shared<ActivityRepository>();
    pImpl_->stockRepo = std::make_shared<StockRepository>();
}

ActivityService::~ActivityService()
{
    delete pImpl_;
}

Result::Ptr ActivityService::createActivity(
    const std::string& name,
    int totalStock,
    double price,
    const std::string& startTime,
    const std::string& endTime)
{
    try {
        // 1. 创建活动
        Activity activity;
        activity.setName(name);
        activity.setTotalStock(totalStock);
        activity.setRemainStock(totalStock);
        activity.setPrice(price);
        activity.setStartTime(startTime);
        activity.setEndTime(endTime);
        activity.setStatus(Activity::NOT_START);

        if (!pImpl_->activityRepo->create(activity)) {
            return Result::fail(ErrorCode::ERR_INTERNAL, "Failed to create activity");
        }

        // 2. 查询刚创建的活动获取ID
        auto activities = pImpl_->activityRepo->findAll();
        if (activities.empty()) {
            return Result::fail(ErrorCode::ERR_INTERNAL, "Activity not found after creation");
        }

        Activity::Ptr newActivity = activities[0];

        // 3. 初始化 Redis 库存
        if (!pImpl_->stockRepo->initStock(newActivity->getId(), totalStock)) {
            return Result::fail(ErrorCode::ERR_STOCK_INIT_FAIL, "Failed to init stock in Redis");
        }

        return Result::success(newActivity->toJson());

    } catch (const std::exception& e) {
        std::cerr << "createActivity exception: " << e.what() << std::endl;
        return Result::fail(ErrorCode::ERR_INTERNAL, e.what());
    }
}

Result::Ptr ActivityService::getActivityList()
{
    try {
        auto activities = pImpl_->activityRepo->findAll();

        Json::Value data;
        for (const auto& activity : activities) {
            data.append(activity->toJson());
        }

        return Result::success(data);

    } catch (const std::exception& e) {
        std::cerr << "getActivityList exception: " << e.what() << std::endl;
        return Result::fail(ErrorCode::ERR_INTERNAL, e.what());
    }
}

Result::Ptr ActivityService::getActivity(long long id)
{
    try {
        auto activity = pImpl_->activityRepo->findById(id);
        if (!activity) {
            return Result::fail(ErrorCode::ERR_ACTIVITY_NOT_FOUND, "Activity not found");
        }

        return Result::success(activity->toJson());

    } catch (const std::exception& e) {
        std::cerr << "getActivity exception: " << e.what() << std::endl;
        return Result::fail(ErrorCode::ERR_INTERNAL, e.what());
    }
}

Result::Ptr ActivityService::getActivityStock(long long id)
{
    try {
        auto activity = pImpl_->activityRepo->findById(id);
        if (!activity) {
            return Result::fail(ErrorCode::ERR_ACTIVITY_NOT_FOUND, "Activity not found");
        }

        int stock = pImpl_->stockRepo->getStock(id);
        if (stock < 0) {
            stock = activity->getRemainStock();
            pImpl_->stockRepo->initStock(id, stock);
        }

        Json::Value data;
        data["remain_stock"] = stock;
        return Result::success(data);

    } catch (const std::exception& e) {
        std::cerr << "getActivityStock exception: " << e.what() << std::endl;
        return Result::fail(ErrorCode::ERR_INTERNAL, e.what());
    }
}

Result::Ptr ActivityService::checkActivityStatus(long long id)
{
    try {
        auto activity = pImpl_->activityRepo->findById(id);
        if (!activity) {
            return Result::fail(ErrorCode::ERR_ACTIVITY_NOT_FOUND, "Activity not found");
        }

        std::time_t now = std::time(nullptr);
        std::tm startTm = {};
        std::tm endTm = {};

        std::istringstream startSs(activity->getStartTime());
        startSs >> std::get_time(&startTm, "%Y-%m-%d %H:%M:%S");
        std::istringstream endSs(activity->getEndTime());
        endSs >> std::get_time(&endTm, "%Y-%m-%d %H:%M:%S");

        std::time_t startTime = std::mktime(&startTm);
        std::time_t endTime = std::mktime(&endTm);

        int status = Activity::NOT_START;
        if (now >= endTime) {
            status = Activity::ENDED;
        } else if (now >= startTime) {
            status = Activity::ACTIVE;
        }

        Json::Value data;
        data["status"] = status;
        return Result::success(data);

    } catch (const std::exception& e) {
        std::cerr << "checkActivityStatus exception: " << e.what() << std::endl;
        return Result::fail(ErrorCode::ERR_INTERNAL, e.what());
    }
}

Result::Ptr ActivityService::getCountdown(long long id)
{
    try {
        auto activity = pImpl_->activityRepo->findById(id);
        if (!activity) {
            return Result::fail(ErrorCode::ERR_ACTIVITY_NOT_FOUND, "Activity not found");
        }

        std::time_t now = std::time(nullptr);
        std::tm startTm = {};
        std::tm endTm = {};

        std::istringstream startSs(activity->getStartTime());
        startSs >> std::get_time(&startTm, "%Y-%m-%d %H:%M:%S");
        std::istringstream endSs(activity->getEndTime());
        endSs >> std::get_time(&endTm, "%Y-%m-%d %H:%M:%S");

        std::time_t startTime = std::mktime(&startTm);
        std::time_t endTime = std::mktime(&endTm);

        int64_t countdown = 0;
        int computedStatus = 0; // 0=未开始, 1=进行中, 2=已结束
        if (now >= endTime) {
            countdown = 0;
            computedStatus = 2; // 已结束
        } else if (now >= startTime) {
            countdown = endTime - now;
            computedStatus = 1; // 进行中
        } else {
            countdown = startTime - now;
            computedStatus = 0; // 未开始
        }

        Json::Value data;
        data["countdown"] = (int64_t)countdown;
        data["start_time"] = activity->getStartTime();
        data["end_time"] = activity->getEndTime();
        data["status"] = computedStatus;

        return Result::success(data);

    } catch (const std::exception& e) {
        std::cerr << "getCountdown exception: " << e.what() << std::endl;
        return Result::fail(ErrorCode::ERR_INTERNAL, e.what());
    }
}

} // namespace seckill
