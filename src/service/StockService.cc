/**
 * @file StockService.cc
 * @brief 库存服务层实现
 */

#include "service/StockService.h"
#include "repository/StockRepository.h"
#include "common/ErrorCode.h"
#include <iostream>

namespace seckill
{

struct StockService::Impl
{
    StockRepository::Ptr stockRepo;
};

StockService::StockService()
{
    pImpl_ = new Impl();
    pImpl_->stockRepo = std::make_shared<StockRepository>();
}

StockService::~StockService()
{
    delete pImpl_;
}

Result::Ptr StockService::initStock(long long activityId, int stock)
{
    try {
        if (pImpl_->stockRepo->initStock(activityId, stock)) {
            return Result::success();
        }
        return Result::fail(ErrorCode::ERR_STOCK_INIT_FAIL, "Failed to init stock");
    } catch (const std::exception& e) {
        std::cerr << "initStock exception: " << e.what() << std::endl;
        return Result::fail(ErrorCode::ERR_INTERNAL, e.what());
    }
}

Result::Ptr StockService::decreaseStock(long long activityId, int quantity)
{
    try {
        int result = pImpl_->stockRepo->decreaseStock(activityId, quantity);

        if (result < 0) {
            if (result == -2) {
                return Result::fail(ErrorCode::ERR_STOCK_NOT_ENOUGH, "Stock not initialized");
            }
            return Result::fail(ErrorCode::ERR_STOCK_NOT_ENOUGH, "Stock not enough");
        }

        Json::Value data;
        data["remain_stock"] = result;
        return Result::success(data);

    } catch (const std::exception& e) {
        std::cerr << "decreaseStock exception: " << e.what() << std::endl;
        return Result::fail(ErrorCode::ERR_INTERNAL, e.what());
    }
}

Result::Ptr StockService::getStock(long long activityId)
{
    try {
        int stock = pImpl_->stockRepo->getStock(activityId);
        if (stock < 0) {
            return Result::fail(ErrorCode::ERR_NOT_FOUND, "Stock not found");
        }

        Json::Value data;
        data["remain_stock"] = stock;
        return Result::success(data);

    } catch (const std::exception& e) {
        std::cerr << "getStock exception: " << e.what() << std::endl;
        return Result::fail(ErrorCode::ERR_INTERNAL, e.what());
    }
}

Result::Ptr StockService::increaseStock(long long activityId, int quantity)
{
    try {
        if (pImpl_->stockRepo->increaseStock(activityId, quantity)) {
            return Result::success();
        }
        return Result::fail(ErrorCode::ERR_INTERNAL, "Failed to increase stock");
    } catch (const std::exception& e) {
        std::cerr << "increaseStock exception: " << e.what() << std::endl;
        return Result::fail(ErrorCode::ERR_INTERNAL, e.what());
    }
}

} // namespace seckill
