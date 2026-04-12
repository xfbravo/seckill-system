/**
 * @file test_connection.cc
 * @brief 测试数据库和 Redis 连接
 */

#include <iostream>
#include <string>
#include <hiredis/hiredis.h>
#include <mysql/mysql.h>

// 数据库配置
const char* DB_HOST = "10.195.242.150";
const char* DB_USER = "root";
const char* DB_PASS = "Dengni0425@xf";
const char* DB_NAME = "seckill_db";
int DB_PORT = 3306;

// Redis 配置
const char* REDIS_HOST = "127.0.0.1";
int REDIS_PORT = 6379;

void testRedis()
{
    std::cout << "\n=== Testing Redis ===" << std::endl;

    redisContext* redis = redisConnect(REDIS_HOST, REDIS_PORT);
    if (redis == nullptr || redis->err) {
        if (redis) {
            std::cerr << "Redis Error: " << redis->errstr << std::endl;
            redisFree(redis);
        } else {
            std::cerr << "Redis: Failed to allocate context" << std::endl;
        }
        return;
    }

    std::cout << "Redis: Connected successfully!" << std::endl;

    // 测试 PING
    redisReply* reply = (redisReply*)redisCommand(redis, "PING");
    if (reply && reply->type == REDIS_REPLY_STRING) {
        std::cout << "Redis: PING -> " << reply->str << std::endl;
    }
    if (reply) freeReplyObject(reply);

    // 测试 SET/GET
    reply = (redisReply*)redisCommand(redis, "SET test_key hello");
    if (reply) freeReplyObject(reply);

    reply = (redisReply*)redisCommand(redis, "GET test_key");
    if (reply && reply->type == REDIS_REPLY_STRING) {
        std::cout << "Redis: GET test_key -> " << reply->str << std::endl;
    }
    if (reply) freeReplyObject(reply);

    redisFree(redis);
    std::cout << "Redis: Test passed!" << std::endl;
}

void testMySQL()
{
    std::cout << "\n=== Testing MySQL ===" << std::endl;

    MYSQL* conn = mysql_init(nullptr);
    if (!conn) {
        std::cerr << "MySQL: Init failed" << std::endl;
        return;
    }

    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS,
                           DB_NAME, DB_PORT, nullptr, 0)) {
        std::cerr << "MySQL Error: " << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return;
    }

    std::cout << "MySQL: Connected successfully!" << std::endl;

    // 测试查询
    if (mysql_query(conn, "SELECT 1")) {
        std::cerr << "MySQL Query Error: " << mysql_error(conn) << std::endl;
    } else {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            std::cout << "MySQL: Query successful!" << std::endl;
            mysql_free_result(res);
        }
    }

    // 检查表
    if (mysql_query(conn, "SHOW TABLES")) {
        std::cerr << "MySQL: SHOW TABLES failed - " << mysql_error(conn) << std::endl;
    } else {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            int num_rows = mysql_num_rows(res);
            std::cout << "MySQL: Found " << num_rows << " tables" << std::endl;
            mysql_free_result(res);
        }
    }

    mysql_close(conn);
    std::cout << "MySQL: Test passed!" << std::endl;
}

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "    Connection Test for Seckill System " << std::endl;
    std::cout << "========================================" << std::endl;

    testRedis();
    testMySQL();

    std::cout << "\n========================================" << std::endl;
    std::cout << "           Test Complete               " << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
