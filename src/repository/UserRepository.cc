/**
 * @file UserRepository.cc
 * @brief 用户数据访问层实现
 */

#include "repository/UserRepository.h"
#include "model/User.h"
#include "common/Config.h"
#include <mysql/mysql.h>
#include <iostream>
#include <sstream>

namespace seckill
{

struct UserRepository::Impl
{
    MYSQL* conn = nullptr;
};

UserRepository::UserRepository()
{
    pImpl_ = new Impl();

    pImpl_->conn = mysql_init(nullptr);
    if (!pImpl_->conn) {
        std::cerr << "MySQL init failed" << std::endl;
        return;
    }

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

    mysql_set_character_set(pImpl_->conn, "utf8mb4");
}

UserRepository::~UserRepository()
{
    if (pImpl_->conn) {
        mysql_close(pImpl_->conn);
        pImpl_->conn = nullptr;
    }
    delete pImpl_;
}

User::Ptr UserRepository::findById(long long id)
{
    if (!pImpl_->conn) return nullptr;

    std::stringstream ss;
    ss << "SELECT id, created_at FROM seckill_user WHERE id = " << id;

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

    auto user = std::make_shared<User>();
    user->setId(atoll(row[0]));
    user->setCreatedAt(row[1] ? row[1] : "");

    mysql_free_result(res);
    return user;
}

bool UserRepository::createIfNotExists(long long id)
{
    if (!pImpl_->conn) return false;

    // 先查询是否存在
    auto existing = findById(id);
    if (existing) {
        return true; // 已存在
    }

    // 创建用户
    std::stringstream ss;
    ss << "INSERT INTO seckill_user (id) VALUES (" << id << ")";

    if (mysql_query(pImpl_->conn, ss.str().c_str())) {
        std::cerr << "Create user failed: " << mysql_error(pImpl_->conn) << std::endl;
        return false;
    }

    return true;
}

std::vector<User::Ptr> UserRepository::findAll()
{
    std::vector<User::Ptr> list;

    if (!pImpl_->conn) return list;

    if (mysql_query(pImpl_->conn, "SELECT id, created_at FROM seckill_user ORDER BY id")) {
        return list;
    }

    MYSQL_RES* res = mysql_store_result(pImpl_->conn);
    if (!res) return list;

    while (MYSQL_ROW row = mysql_fetch_row(res)) {
        auto user = std::make_shared<User>();
        user->setId(atoll(row[0]));
        user->setCreatedAt(row[1] ? row[1] : "");
        list.push_back(user);
    }

    mysql_free_result(res);
    return list;
}

} // namespace seckill
