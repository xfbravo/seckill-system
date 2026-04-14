/**
 * @file ActivityRepository.cc
 * @brief 活动数据访问层实现
 *
 */

#include "repository/ActivityRepository.h"
#include "model/Activity.h"
#include "common/Config.h"
#include <mysql/mysql.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <mutex>

namespace seckill
{

struct ActivityRepository::Impl
{
    MYSQL* conn = nullptr;
    std::mutex connMutex;  // Protects MySQL connection
};

ActivityRepository::ActivityRepository()
{
    pImpl_ = new Impl();

    // 初始化 MySQL 连接
    pImpl_->conn = mysql_init(nullptr);
    if (!pImpl_->conn) {
        std::cerr << "MySQL init failed" << std::endl;
        return;
    }

    // 从配置读取数据库连接信息
    Config& cfg = Config::instance();
    if (!mysql_real_connect(pImpl_->conn,
                           cfg.dbHost().c_str(),
                           cfg.dbUser().c_str(),
                           cfg.dbPass().c_str(),
                           cfg.dbName().c_str(),
                           cfg.dbPort(),
                           nullptr, 0)) {
        std::cerr << "MySQL connection failed: " << mysql_error(pImpl_->conn) << std::endl;
        mysql_close(pImpl_->conn);
        pImpl_->conn = nullptr;
        return;
    }

    // 设置字符集
    mysql_set_character_set(pImpl_->conn, "utf8mb4");
}

ActivityRepository::~ActivityRepository()
{
    if (pImpl_->conn) {
        mysql_close(pImpl_->conn);
        pImpl_->conn = nullptr;
    }
    delete pImpl_;
}

bool ActivityRepository::create(const Activity& activity)
{
    if (!pImpl_->conn) return false;

    std::stringstream ss;
    ss << "INSERT INTO seckill_activity (name, total_stock, remain_stock, price, start_time, end_time, status) VALUES ("
       << "'" << activity.getName() << "', "
       << activity.getTotalStock() << ", "
       << activity.getRemainStock() << ", "
       << activity.getPrice() << ", "
       << "'" << activity.getStartTime() << "', "
       << "'" << activity.getEndTime() << "', "
       << activity.getStatus() << ")";

    bool success = false;
    {
        std::lock_guard<std::mutex> lock(pImpl_->connMutex);
        success = (mysql_query(pImpl_->conn, ss.str().c_str()) == 0);
    }
    if (!success) {
        std::cerr << "Insert failed: " << mysql_error(pImpl_->conn) << std::endl;
        return false;
    }

    return true;
}

Activity::Ptr ActivityRepository::findById(long long id)
{
    if (!pImpl_->conn) return nullptr;

    std::stringstream ss;
    ss << "SELECT id, name, total_stock, remain_stock, price, start_time, end_time, status, created_at FROM seckill_activity WHERE id = " << id;

    Activity::Ptr activity;
    {
        std::lock_guard<std::mutex> lock(pImpl_->connMutex);
        if (mysql_query(pImpl_->conn, ss.str().c_str())) {
            return nullptr;
        }

        MYSQL_RES* res = mysql_store_result(pImpl_->conn);
        if (!res || mysql_num_rows(res) == 0) {
            if (res) mysql_free_result(res);
            return nullptr;
        }

        MYSQL_ROW row = mysql_fetch_row(res);
        if (!row) {
            mysql_free_result(res);
            return nullptr;
        }

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

        mysql_free_result(res);
    }
    return activity;
}

std::vector<Activity::Ptr> ActivityRepository::findAll()
{
    std::vector<Activity::Ptr> list;

    if (!pImpl_->conn) return list;

    {
        std::lock_guard<std::mutex> lock(pImpl_->connMutex);
        if (mysql_query(pImpl_->conn, "SELECT id, name, total_stock, remain_stock, price, start_time, end_time, status, created_at FROM seckill_activity ORDER BY id DESC")) {
            return list;
        }

        MYSQL_RES* res = mysql_store_result(pImpl_->conn);
        if (!res) return list;

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
    return list;
}

bool ActivityRepository::update(const Activity& activity)
{
    if (!pImpl_->conn) return false;

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

    bool success = false;
    {
        std::lock_guard<std::mutex> lock(pImpl_->connMutex);
        success = (mysql_query(pImpl_->conn, ss.str().c_str()) == 0);
    }
    return success;
}

bool ActivityRepository::updateRemainStock(long long id, int remainStock)
{
    if (!pImpl_->conn) return false;

    std::stringstream ss;
    ss << "UPDATE seckill_activity SET remain_stock = " << remainStock << " WHERE id = " << id;

    bool success = false;
    {
        std::lock_guard<std::mutex> lock(pImpl_->connMutex);
        success = (mysql_query(pImpl_->conn, ss.str().c_str()) == 0);
    }
    return success;
}

bool ActivityRepository::remove(long long id)
{
    if (!pImpl_->conn) return false;

    std::stringstream ss;
    ss << "DELETE FROM seckill_activity WHERE id = " << id;

    bool success = false;
    {
        std::lock_guard<std::mutex> lock(pImpl_->connMutex);
        success = (mysql_query(pImpl_->conn, ss.str().c_str()) == 0);
    }
    return success;
}

} // namespace seckill
