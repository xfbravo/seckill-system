/**
 * @file OrderRepository.cc
 * @brief 订单数据访问层实现
 */

#include "repository/OrderRepository.h"
#include "model/Order.h"
#include "common/Config.h"
#include <mysql/mysql.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <mutex>

namespace seckill
{

struct OrderRepository::Impl
{
    MYSQL* conn = nullptr;
    std::mutex connMutex;  // Protects MySQL connection
};

OrderRepository::OrderRepository()
{
    pImpl_ = new Impl();

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
    }

    mysql_set_character_set(pImpl_->conn, "utf8mb4");
}

OrderRepository::~OrderRepository()
{
    if (pImpl_->conn) {
        mysql_close(pImpl_->conn);
        pImpl_->conn = nullptr;
    }
    delete pImpl_;
}

bool OrderRepository::create(const Order& order)
{
    if (!pImpl_->conn) return false;

    std::stringstream ss;
    ss << "INSERT INTO seckill_order (order_no, activity_id, user_id, quantity, status) VALUES ("
       << "'" << order.getOrderNo() << "', "
       << order.getActivityId() << ", "
       << order.getUserId() << ", "
       << order.getQuantity() << ", "
       << order.getStatus() << ")";

    bool success = false;
    {
        std::lock_guard<std::mutex> lock(pImpl_->connMutex);
        success = (mysql_query(pImpl_->conn, ss.str().c_str()) == 0);
    }
    if (!success) {
        std::cerr << "Insert order failed: " << mysql_error(pImpl_->conn) << std::endl;
    }
    return success;
}

bool OrderRepository::createBatch(const std::vector<Order>& orders)
{
    if (!pImpl_->conn || orders.empty()) return false;

    std::stringstream ss;
    ss << "INSERT INTO seckill_order (order_no, activity_id, user_id, quantity, status) VALUES ";
    for (size_t i = 0; i < orders.size(); ++i) {
        if (i > 0) ss << ", ";
        const auto& order = orders[i];
        ss << "('" << order.getOrderNo() << "', "
           << order.getActivityId() << ", "
           << order.getUserId() << ", "
           << order.getQuantity() << ", "
           << order.getStatus() << ")";
    }

    bool success = false;
    {
        std::lock_guard<std::mutex> lock(pImpl_->connMutex);
        success = (mysql_query(pImpl_->conn, ss.str().c_str()) == 0);
    }
    if (!success) {
        std::cerr << "Batch insert orders failed: " << mysql_error(pImpl_->conn) << std::endl;
    }
    return success;
}

Order::Ptr OrderRepository::findById(long long id)
{
    if (!pImpl_->conn) return nullptr;

    std::stringstream ss;
    ss << "SELECT id, order_no, activity_id, user_id, quantity, status, created_at FROM seckill_order WHERE id = " << id;

    Order::Ptr order;
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

        order = std::make_shared<Order>();
        order->setId(atoll(row[0]));
        order->setOrderNo(row[1] ? row[1] : "");
        order->setActivityId(atoll(row[2]));
        order->setUserId(atoll(row[3]));
        order->setQuantity(atoi(row[4]));
        order->setStatus(atoi(row[5]));
        order->setCreatedAt(row[6] ? row[6] : "");

        mysql_free_result(res);
    }
    return order;
}

Order::Ptr OrderRepository::findByOrderNo(const std::string& orderNo)
{
    if (!pImpl_->conn) return nullptr;

    std::stringstream ss;
    ss << "SELECT id, order_no, activity_id, user_id, quantity, status, created_at FROM seckill_order WHERE order_no = '" << orderNo << "'";

    Order::Ptr order;
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

        order = std::make_shared<Order>();
        order->setId(atoll(row[0]));
        order->setOrderNo(row[1] ? row[1] : "");
        order->setActivityId(atoll(row[2]));
        order->setUserId(atoll(row[3]));
        order->setQuantity(atoi(row[4]));
        order->setStatus(atoi(row[5]));
        order->setCreatedAt(row[6] ? row[6] : "");

        mysql_free_result(res);
    }
    return order;
}

std::vector<Order::Ptr> OrderRepository::findByUserId(long long userId)
{
    std::vector<Order::Ptr> list;

    if (!pImpl_->conn) return list;

    std::stringstream ss;
    ss << "SELECT id, order_no, activity_id, user_id, quantity, status, created_at FROM seckill_order WHERE user_id = " << userId << " ORDER BY id DESC";

    {
        std::lock_guard<std::mutex> lock(pImpl_->connMutex);
        if (mysql_query(pImpl_->conn, ss.str().c_str())) {
            return list;
        }

        MYSQL_RES* res = mysql_store_result(pImpl_->conn);
        if (!res) return list;

        while (MYSQL_ROW row = mysql_fetch_row(res)) {
            auto order = std::make_shared<Order>();
            order->setId(atoll(row[0]));
            order->setOrderNo(row[1] ? row[1] : "");
            order->setActivityId(atoll(row[2]));
            order->setUserId(atoll(row[3]));
            order->setQuantity(atoi(row[4]));
            order->setStatus(atoi(row[5]));
            order->setCreatedAt(row[6] ? row[6] : "");
            list.push_back(order);
        }

        mysql_free_result(res);
    }
    return list;
}

std::vector<Order::Ptr> OrderRepository::findByActivityId(long long activityId)
{
    std::vector<Order::Ptr> list;

    if (!pImpl_->conn) return list;

    std::stringstream ss;
    ss << "SELECT id, order_no, activity_id, user_id, quantity, status, created_at FROM seckill_order WHERE activity_id = " << activityId << " ORDER BY id DESC";

    {
        std::lock_guard<std::mutex> lock(pImpl_->connMutex);
        if (mysql_query(pImpl_->conn, ss.str().c_str())) {
            return list;
        }

        MYSQL_RES* res = mysql_store_result(pImpl_->conn);
        if (!res) return list;

        while (MYSQL_ROW row = mysql_fetch_row(res)) {
            auto order = std::make_shared<Order>();
            order->setId(atoll(row[0]));
            order->setOrderNo(row[1] ? row[1] : "");
            order->setActivityId(atoll(row[2]));
            order->setUserId(atoll(row[3]));
            order->setQuantity(atoi(row[4]));
            order->setStatus(atoi(row[5]));
            order->setCreatedAt(row[6] ? row[6] : "");
            list.push_back(order);
        }

        mysql_free_result(res);
    }
    return list;
}

bool OrderRepository::updateStatus(long long id, int status)
{
    if (!pImpl_->conn) return false;

    std::stringstream ss;
    ss << "UPDATE seckill_order SET status = " << status << " WHERE id = " << id;

    bool success = false;
    {
        std::lock_guard<std::mutex> lock(pImpl_->connMutex);
        success = (mysql_query(pImpl_->conn, ss.str().c_str()) == 0);
    }
    return success;
}

std::vector<Order::Ptr> OrderRepository::findAll()
{
    std::vector<Order::Ptr> list;

    if (!pImpl_->conn) return list;

    {
        std::lock_guard<std::mutex> lock(pImpl_->connMutex);
        if (mysql_query(pImpl_->conn, "SELECT id, order_no, activity_id, user_id, quantity, status, created_at FROM seckill_order ORDER BY id DESC")) {
            return list;
        }

        MYSQL_RES* res = mysql_store_result(pImpl_->conn);
        if (!res) return list;

        while (MYSQL_ROW row = mysql_fetch_row(res)) {
            auto order = std::make_shared<Order>();
            order->setId(atoll(row[0]));
            order->setOrderNo(row[1] ? row[1] : "");
            order->setActivityId(atoll(row[2]));
            order->setUserId(atoll(row[3]));
            order->setQuantity(atoi(row[4]));
            order->setStatus(atoi(row[5]));
            order->setCreatedAt(row[6] ? row[6] : "");
            list.push_back(order);
        }

        mysql_free_result(res);
    }
    return list;
}

} // namespace seckill
