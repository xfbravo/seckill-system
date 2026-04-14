/**
 * @file ActivityRepository.cc
 * @brief 活动数据访问层实现 - 使用 MySQL 连接池
 */

#include "repository/ActivityRepository.h"
#include "model/Activity.h"
#include "common/Config.h"
#include "common/MysqlPool.h"
#include <mysql/mysql.h>
#include <iostream>
#include <sstream>
#include <vector>

namespace seckill
{

struct ActivityRepository::Impl
{
    MysqlPool::Ptr mysqlPool;
};

ActivityRepository::ActivityRepository()
{
    pImpl_ = new Impl();
    Config& cfg = Config::instance();
    pImpl_->mysqlPool = std::make_shared<MysqlPool>(
        cfg.dbHost(), cfg.dbUser(), cfg.dbPass(),
        cfg.dbName(), cfg.dbPort(), 10);
}

ActivityRepository::~ActivityRepository() = default;

bool ActivityRepository::create(const Activity& activity)
{
    MYSQL* conn = pImpl_->mysqlPool->get();
    if (!conn) return false;

    std::stringstream ss;
    ss << "INSERT INTO seckill_activity (name, total_stock, remain_stock, price, start_time, end_time, status) VALUES ("
       << "'" << activity.getName() << "', "
       << activity.getTotalStock() << ", "
       << activity.getRemainStock() << ", "
       << activity.getPrice() << ", "
       << "'" << activity.getStartTime() << "', "
       << "'" << activity.getEndTime() << "', "
       << activity.getStatus() << ")";

    bool success = (mysql_query(conn, ss.str().c_str()) == 0);
    if (!success) {
        std::cerr << "Insert failed: " << mysql_error(conn) << std::endl;
    }
    pImpl_->mysqlPool->release(conn);
    return success;
}

Activity::Ptr ActivityRepository::findById(long long id)
{
    MYSQL* conn = pImpl_->mysqlPool->get();
    if (!conn) return nullptr;

    std::stringstream ss;
    ss << "SELECT id, name, total_stock, remain_stock, price, start_time, end_time, status, created_at FROM seckill_activity WHERE id = " << id;

    Activity::Ptr activity;
    if (mysql_query(conn, ss.str().c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res && mysql_num_rows(res) > 0) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row) {
                activity = std::make_shared<Activity>();
                activity->setId(atoll(row[0]));
                activity->setName(row[1] ? row[1] : "");
                activity->setTotalStock(atoi(row[2]));
                activity->setRemainStock(atoi(row[3]));
                activity->setPrice(atof(row[4]));
                activity->setStartTime(row[5] ? row[5] : "");
                activity->setEndTime(row[6] ? row[6] : "");
                activity->setStatus(atoi(row[7]));
                activity->setCreatedAt(row[8] ? row[8] : "");
            }
            mysql_free_result(res);
        } else {
            if (res) mysql_free_result(res);
        }
    }
    pImpl_->mysqlPool->release(conn);
    return activity;
}

std::vector<Activity::Ptr> ActivityRepository::findAll()
{
    std::vector<Activity::Ptr> list;

    MYSQL* conn = pImpl_->mysqlPool->get();
    if (!conn) return list;

    if (mysql_query(conn, "SELECT id, name, total_stock, remain_stock, price, start_time, end_time, status, created_at FROM seckill_activity ORDER BY id DESC") == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            while (MYSQL_ROW row = mysql_fetch_row(res)) {
                auto activity = std::make_shared<Activity>();
                activity->setId(atoll(row[0]));
                activity->setName(row[1] ? row[1] : "");
                activity->setTotalStock(atoi(row[2]));
                activity->setRemainStock(atoi(row[3]));
                activity->setPrice(atof(row[4]));
                activity->setStartTime(row[5] ? row[5] : "");
                activity->setEndTime(row[6] ? row[6] : "");
                activity->setStatus(atoi(row[7]));
                activity->setCreatedAt(row[8] ? row[8] : "");
                list.push_back(activity);
            }
            mysql_free_result(res);
        }
    }
    pImpl_->mysqlPool->release(conn);
    return list;
}

bool ActivityRepository::update(const Activity& activity)
{
    MYSQL* conn = pImpl_->mysqlPool->get();
    if (!conn) return false;

    std::stringstream ss;
    ss << "UPDATE seckill_activity SET "
       << "name = '" << activity.getName() << "', "
       << "total_stock = " << activity.getTotalStock() << ", "
       << "remain_stock = " << activity.getRemainStock() << ", "
       << "price = " << activity.getPrice() << ", "
       << "start_time = '" << activity.getStartTime() << "', "
       << "end_time = '" << activity.getEndTime() << "', "
       << "status = " << activity.getStatus()
       << " WHERE id = " << activity.getId();

    bool success = (mysql_query(conn, ss.str().c_str()) == 0);
    pImpl_->mysqlPool->release(conn);
    return success;
}

bool ActivityRepository::updateRemainStock(long long id, int remainStock)
{
    MYSQL* conn = pImpl_->mysqlPool->get();
    if (!conn) return false;

    std::stringstream ss;
    ss << "UPDATE seckill_activity SET remain_stock = " << remainStock << " WHERE id = " << id;

    bool success = (mysql_query(conn, ss.str().c_str()) == 0);
    pImpl_->mysqlPool->release(conn);
    return success;
}

bool ActivityRepository::remove(long long id)
{
    MYSQL* conn = pImpl_->mysqlPool->get();
    if (!conn) return false;

    std::stringstream ss;
    ss << "DELETE FROM seckill_activity WHERE id = " << id;

    bool success = (mysql_query(conn, ss.str().c_str()) == 0);
    pImpl_->mysqlPool->release(conn);
    return success;
}

} // namespace seckill