# 秒杀系统 (Seckill System)

高并发秒杀系统，支持 100,000 并发请求，防超卖。

## 快速启动

### 1. 启动 MySQL 和 Redis

确保 MySQL (your IP : port) 和 Redis 服务运行中。

### 2. 启动后端服务器

```bash
cd /home/xf/桌面/Buy/seckill-system/build
./seckill_server
```

服务器默认运行在 `http://localhost:8080`

### 3. 启动 Qt 客户端

运行编译好的 `seckill_client.exe`（Windows）或 `seckill_client`（Linux）。

## 添加新秒杀商品

### 方式一：API 接口（推荐）

```bash
curl -X POST http://localhost:8080/api/activity/create \
  -H "Content-Type: application/json" \
  -d '{
    "name": "商品名称",
    "total_stock": 1000,
    "price": 2999.00,
    "start_time": "2026-04-15 10:00:00",
    "end_time": "2026-04-15 12:00:00"
  }'
```

### 方式二：直接操作数据库

```sql
USE seckill_db;

INSERT INTO seckill_activity (name, total_stock, remain_stock, price, start_time, end_time, status)
VALUES ('商品名称', 1000, 1000, 2999.00, '2026-04-15 10:00:00', '2026-04-15 12:00:00', 0);
```

**注意**：`remain_stock` 必须与 `total_stock` 相同，库存会在 Redis 中独立管理。

## API 接口列

| 接口 | 方法 | 说明 |
|------|------|------|
| `/api/activity/list` | GET | 获取活动列表 |
| `/api/activity/{id}` | GET | 获取活动详情 |
| `/api/activity/{id}/stock` | GET | 获取实时库存 |
| `/api/seckill/countdown/{id}` | GET | 获取倒计时和状态 |
| `/api/seckill/order` | POST | 提交秒杀订单 |
| `/api/order/{id}` | GET | 查询订单 |
| `/api/user/login` | POST | 用户登录 |

## 项目结构

```
seckill-system/
├── src/                    # 后端源代码
│   ├── controller/         # 控制器层
│   ├── service/            # 服务层
│   └── repository/         # 数据访问层
├── include/                # 头文件
├── client/                 # Qt 客户端
├── scripts/                # 数据库脚本
└── build/                  # 编译输出目录
```
