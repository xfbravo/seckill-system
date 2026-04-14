/**
 * @file OrderRepository.cc
 * @brief 订单数据访问层实现 - 使用 MySQL 连接池
 */

#include "repository/OrderRepository.h"
#include "model/Order.h"
#include "common/Config.h"
#include "common/MysqlPool.h"
#include <mysql/mysql.h>
#include <iostream>
#include <sstream>
#include <vector>

namespace seckill
{

struct OrderRepository::Impl
{
    MysqlPool::Ptr mysqlPool;
};

OrderRepository::OrderRepository()
{
    pImpl_ = new Impl();
    Config& cfg = Config::instance();
    pImpl_->mysqlPool = std::make_shared<MysqlPool>(
        cfg.dbHost(), cfg.dbUser(), cfg.dbPass(),
        cfg.dbName(), cfg.dbPort(), 10);
}

OrderRepository::~OrderRepository() = default;

bool OrderRepository::create(const Order& order)
{
    MYSQL* conn = pImpl_->mysqlPool->get();
    if (!conn) return false;

    std::stringstream ss;
    ss << "INSERT INTO seckill_order (order_no, activity_id, user_id, quantity, status) VALUES ("
       << "'" << order.getOrderNo() << "', "
       << order.getActivityId() << ", "
       << order.getUserId() << ", "
       << order.getQuantity() << ", "
       << order.getStatus() << ")";

    bool success = (mysql_query(conn, ss.str().c_str()) == 0);
    if (!success) {
        std::cerr << "Insert order failed: " << mysql_error(conn) << std::endl;
    }
    pImpl_->mysqlPool->release(conn);
    return success;
}

bool OrderRepository::createBatch(const std::vector<Order>& orders)
{
    if (orders.empty()) return false;

    MYSQL* conn = pImpl_->mysqlPool->get();
    if (!conn) return false;

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

    bool success = (mysql_query(conn, ss.str().c_str()) == 0);
    if (!success) {
        std::cerr << "Batch insert orders failed: " << mysql_error(conn) << std::endl;
    }
    pImpl_->mysqlPool->release(conn);
    return success;
}

Order::Ptr OrderRepository::findById(long long id)
{
    MYSQL* conn = pImpl_->mysqlPool->get();
    if (!conn) return nullptr;

    std::stringstream ss;
    ss << "SELECT id, order_no, activity_id, user_id, quantity, status, created_at FROM seckill_order WHERE id = " << id;

    Order::Ptr order;
    if (mysql_query(conn, ss.str().c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res && mysql_num_rows(res) > 0) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row) {
                order = std::make_shared<Order>();
                order->setId(atoll(row[0]));
                order->setOrderNo(row[1] ? row[1] : "");
                order->setActivityId(atoll(row[2]));
                order->setUserId(atoll(row[3]));
                order->setQuantity(atoi(row[4]));
                order->setStatus(atoi(row[5]));
                order->setCreatedAt(row[6] ? row[6] : "");
            }
            mysql_free_result(res);
        } else {
            if (res) mysql_free_result(res);
        }
    }
    pImpl_->mysqlPool->release(conn);
    return order;
}

Order::Ptr OrderRepository::findByOrderNo(const std::string& orderNo)
{
    MYSQL* conn = pImpl_->mysqlPool->get();
    if (!conn) return nullptr;

    std::stringstream ss;
    ss << "SELECT id, order_no, activity_id, user_id, quantity, status, created_at FROM seckill_order WHERE order_no = '" << orderNo << "'";

    Order::Ptr order;
    if (mysql_query(conn, ss.str().c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res && mysql_num_rows(res) > 0) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row) {
                order = std::make_shared<Order>();
                order->setId(atoll(row[0]));
                order->setOrderNo(row[1] ? row[1] : "");
                order->setActivityId(atoll(row[2]));
                order->setUserId(atoll(row[3]));
                order->setQuantity(atoi(row[4]));
                order->setStatus(atoi(row[5]));
                order->setCreatedAt(row[6] ? row[6] : "");
            }
            mysql_free_result(res);
        } else {
            if (res) mysql_free_result(res);
        }
    }
    pImpl_->mysqlPool->release(conn);
    return order;
}

std::vector<Order::Ptr> OrderRepository::findByUserId(long long userId)
{
    std::vector<Order::Ptr> list;

    MYSQL* conn = pImpl_->mysqlPool->get();
    if (!conn) return list;

    std::stringstream ss;
    ss << "SELECT id, order_no, activity_id, user_id, quantity, status, created_at FROM seckill_order WHERE user_id = " << userId << " ORDER BY id DESC";

    if (mysql_query(conn, ss.str().c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
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
    }
    pImpl_->mysqlPool->release(conn);
    return list;
}

std::vector<Order::Ptr> OrderRepository::findByActivityId(long long activityId)
{
    std::vector<Order::Ptr> list;

    MYSQL* conn = pImpl_->mysqlPool->get();
    if (!conn) return list;

    std::stringstream ss;
    ss << "SELECT id, order_no, activity_id, user_id, quantity, status, created_at FROM seckill_order WHERE activity_id = " << activityId << " ORDER BY id DESC";

    if (mysql_query(conn, ss.str().c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
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
    }
    pImpl_->mysqlPool->release(conn);
    return list;
}

bool OrderRepository::updateStatus(long long id, int status)
{
    MYSQL* conn = pImpl_->mysqlPool->get();
    if (!conn) return false;

    std::stringstream ss;
    ss << "UPDATE seckill_order SET status = " << status << " WHERE id = " << id;

    bool success = (mysql_query(conn, ss.str().c_str()) == 0);
    pImpl_->mysqlPool->release(conn);
    return success;
}

std::vector<Order::Ptr> OrderRepository::findAll()
{
    std::vector<Order::Ptr> list;

    MYSQL* conn = pImpl_->mysqlPool->get();
    if (!conn) return list;

    if (mysql_query(conn, "SELECT id, order_no, activity_id, user_id, quantity, status, created_at FROM seckill_order ORDER BY id DESC") == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
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
    }
    pImpl_->mysqlPool->release(conn);
    return list;
}

} // namespace seckill