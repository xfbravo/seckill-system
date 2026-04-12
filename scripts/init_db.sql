-- 秒杀系统数据库初始化脚本
-- 创建数据库

CREATE DATABASE IF NOT EXISTS seckill_db DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

USE seckill_db;

-- 秒杀活动表
CREATE TABLE IF NOT EXISTS seckill_activity (
    id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '活动ID',
    name VARCHAR(128) NOT NULL COMMENT '活动名称',
    total_stock INT NOT NULL COMMENT '总库存',
    remain_stock INT NOT NULL COMMENT '剩余库存',
    price DECIMAL(10,2) NOT NULL COMMENT '秒杀价格',
    start_time DATETIME NOT NULL COMMENT '开始时间',
    end_time DATETIME NOT NULL COMMENT '结束时间',
    status TINYINT DEFAULT 0 COMMENT '状态:0-未开始,1-进行中,2-已结束',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    INDEX idx_start_time (start_time),
    INDEX idx_status (status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='秒杀活动表';

-- 订单表
CREATE TABLE IF NOT EXISTS seckill_order (
    id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '订单ID',
    order_no VARCHAR(64) UNIQUE NOT NULL COMMENT '订单号',
    activity_id BIGINT NOT NULL COMMENT '活动ID',
    user_id BIGINT NOT NULL COMMENT '用户ID',
    quantity INT NOT NULL DEFAULT 1 COMMENT '购买数量',
    status TINYINT DEFAULT 0 COMMENT '状态:0-待确认,1-已确认,2-已取消',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    INDEX idx_activity_id (activity_id),
    INDEX idx_user_id (user_id),
    INDEX idx_order_no (order_no),
    INDEX idx_status (status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='秒杀订单表';

-- 插入测试数据
INSERT INTO seckill_activity (name, total_stock, remain_stock, price, start_time, end_time, status) VALUES
('iPhone 15 秒杀', 100, 100, 5999.00, '2026-04-15 10:00:00', '2026-04-15 12:00:00', 0),
('MacBook Pro 秒杀', 50, 50, 9999.00, '2026-04-15 14:00:00', '2026-04-15 16:00:00', 0),
('AirPods Pro 秒杀', 200, 200, 1499.00, '2026-04-15 18:00:00', '2026-04-15 20:00:00', 0);
