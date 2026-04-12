/**
 * @file Config.h
 * @brief 配置读取模块
 *
 * 为什么这样设计？
 * - 集中管理配置，避免硬编码
 * - 支持从 config.yaml 读取
 * - 便于部署时修改
 */

#pragma once

#include <string>
#include <iostream>
#include <yaml-cpp/yaml.h>

namespace seckill
{

class Config
{
public:
    static Config& instance()
    {
        static Config inst;
        return inst;
    }

    // 加载配置文件
    bool load(const std::string& path = "config.yaml")
    {
        try {
            YAML::Node node = YAML::LoadFile(path);

            // 数据库配置
            dbHost_ = node["database"]["host"].as<std::string>();
            dbPort_ = node["database"]["port"].as<int>();
            dbName_ = node["database"]["database"].as<std::string>();
            dbUser_ = node["database"]["username"].as<std::string>();
            dbPass_ = node["database"]["password"].as<std::string>();
            dbPoolSize_ = node["database"]["pool_size"].as<int>();

            // Redis 配置
            redisHost_ = node["redis"]["host"].as<std::string>();
            redisPort_ = node["redis"]["port"].as<int>();
            redisDb_ = node["redis"]["db"].as<int>();
            redisPoolSize_ = node["redis"]["pool_size"].as<int>();

            // 应用配置
            appPort_ = node["app"]["port"].as<int>();

            return true;
        } catch (const std::exception& e) {
            std::cerr << "Config load error: " << e.what() << std::endl;
            return false;
        } catch (...) {
            return false;
        }
    }

    // 数据库配置
    std::string dbHost() const { return dbHost_; }
    int dbPort() const { return dbPort_; }
    std::string dbName() const { return dbName_; }
    std::string dbUser() const { return dbUser_; }
    std::string dbPass() const { return dbPass_; }
    int dbPoolSize() const { return dbPoolSize_; }

    // Redis 配置
    std::string redisHost() const { return redisHost_; }
    int redisPort() const { return redisPort_; }
    int redisDb() const { return redisDb_; }
    int redisPoolSize() const { return redisPoolSize_; }

    // 应用配置
    int appPort() const { return appPort_; }

private:
    Config() = default;

    // 数据库配置
    std::string dbHost_ = "127.0.0.1";
    int dbPort_ = 3306;
    std::string dbName_ = "seckill_db";
    std::string dbUser_ = "root";
    std::string dbPass_ = "";
    int dbPoolSize_ = 10;

    // Redis 配置
    std::string redisHost_ = "127.0.0.1";
    int redisPort_ = 6379;
    int redisDb_ = 0;
    int redisPoolSize_ = 10;

    // 应用配置
    int appPort_ = 8080;
};

} // namespace seckill