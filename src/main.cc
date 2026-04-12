/**
 * @file main.cc
 * @brief 秒杀系统服务端入口 - 使用 cpp-httplib
 */

#include "http_server.h"
#include "common/Config.h"
#include <iostream>
#include <thread>

int main()
{
    // 加载配置
    seckill::Config::instance().load("config.yaml");

    seckill::Config& cfg = seckill::Config::instance();

    std::cout << "========================================" << std::endl;
    std::cout << "     高并发秒杀系统 - 服务端启动        " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "监听端口: " << cfg.appPort() << std::endl;
    std::cout << "数据库: " << cfg.dbHost() << ":" << cfg.dbPort() << "/" << cfg.dbName() << std::endl;
    std::cout << "Redis: " << cfg.redisHost() << ":" << cfg.redisPort() << std::endl;
    std::cout << "========================================" << std::endl;

    // 创建并启动 HTTP 服务器
    seckill::HttpServer server(cfg.appPort());
    server.start();

    return 0;
}
