/**
 * @file ActivityRepository.h
 * @brief 活动数据访问层
 *
 * 为什么这样设计？
 * - 封装 MySQL 活动表操作
 * - 提供 CRUD 基本操作
 * - 使用原生 SQL，便于性能调优
 */

#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include "model/Activity.h"

namespace seckill
{

class ActivityRepository
{
public:
    using Ptr = std::shared_ptr<ActivityRepository>;

    ActivityRepository();
    ~ActivityRepository();

    // 创建活动
    bool create(const Activity& activity);

    // 根据ID查询活动
    Activity::Ptr findById(long long id);

    // 查询所有活动
    std::vector<Activity::Ptr> findAll();

    // 更新活动
    bool update(const Activity& activity);

    // 更新剩余库存
    bool updateRemainStock(long long id, int remainStock);

    // 删除活动
    bool remove(long long id);

private:
    // Pimpl 模式，隐藏 MySQL 依赖
    struct Impl;
    Impl* pImpl_;
};

} // namespace seckill
