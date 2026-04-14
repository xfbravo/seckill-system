/**
 * @file UserRepository.cc
 * @brief 用户数据访问层实现 - 使用 MySQL 连接池
 */

#include "repository/UserRepository.h"
#include "model/User.h"
#include "common/Config.h"
#include "common/MysqlPool.h"
#include <mysql/mysql.h>
#include <iostream>
#include <sstream>
#include <vector>

namespace seckill
{

struct UserRepository::Impl
{
    MysqlPool::Ptr mysqlPool;
};

UserRepository::UserRepository()
{
    pImpl_ = new Impl();
    Config& cfg = Config::instance();
    pImpl_->mysqlPool = std::make_shared<MysqlPool>(
        cfg.dbHost(), cfg.dbUser(), cfg.dbPass(),
        cfg.dbName(), cfg.dbPort(), 10);
}

UserRepository::~UserRepository() = default;

User::Ptr UserRepository::findById(long long id)
{
    MYSQL* conn = pImpl_->mysqlPool->get();
    if (!conn) return nullptr;

    std::stringstream ss;
    ss << "SELECT id, created_at FROM seckill_user WHERE id = " << id;

    User::Ptr user;
    if (mysql_query(conn, ss.str().c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res && mysql_num_rows(res) > 0) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row) {
                user = std::make_shared<User>();
                user->setId(atoll(row[0]));
                user->setCreatedAt(row[1] ? row[1] : "");
            }
            mysql_free_result(res);
        } else {
            if (res) mysql_free_result(res);
        }
    }
    pImpl_->mysqlPool->release(conn);
    return user;
}

bool UserRepository::createIfNotExists(long long id)
{
    MYSQL* conn = pImpl_->mysqlPool->get();
    if (!conn) return false;

    // 先查询是否存在
    {
        std::stringstream ss;
        ss << "SELECT id, created_at FROM seckill_user WHERE id = " << id;

        if (mysql_query(conn, ss.str().c_str()) == 0) {
            MYSQL_RES* res = mysql_store_result(conn);
            if (res && mysql_num_rows(res) > 0) {
                mysql_free_result(res);
                pImpl_->mysqlPool->release(conn);
                return true; // 已存在
            }
            if (res) mysql_free_result(res);
        }
    }

    // 创建用户
    std::stringstream ss;
    ss << "INSERT INTO seckill_user (id) VALUES (" << id << ")";

    bool success = (mysql_query(conn, ss.str().c_str()) == 0);
    if (!success) {
        std::cerr << "Create user failed: " << mysql_error(conn) << std::endl;
    }
    pImpl_->mysqlPool->release(conn);
    return success;
}

std::vector<User::Ptr> UserRepository::findAll()
{
    std::vector<User::Ptr> list;

    MYSQL* conn = pImpl_->mysqlPool->get();
    if (!conn) return list;

    if (mysql_query(conn, "SELECT id, created_at FROM seckill_user ORDER BY id") == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            while (MYSQL_ROW row = mysql_fetch_row(res)) {
                auto user = std::make_shared<User>();
                user->setId(atoll(row[0]));
                user->setCreatedAt(row[1] ? row[1] : "");
                list.push_back(user);
            }
            mysql_free_result(res);
        }
    }
    pImpl_->mysqlPool->release(conn);
    return list;
}

} // namespace seckill