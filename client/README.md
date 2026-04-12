# 秒杀系统 Qt 客户端

## 目录结构

```
client/
├── seckill_client.pro       # Qt 项目文件
├── main.cc                  # 程序入口
├── MainWindow.h/cc          # 主界面（活动列表）
├── ActivityDetailWindow.h/cc # 活动详情页（含倒计时和抢购）
└── NetworkManager.h/cc      # HTTP 网络请求管理
```

## 在 Windows 上构建

### 1. 安装 Qt Creator
- 下载 Qt Creator: https://www.qt.io/download
- 安装时选择 Qt 6.x 和 MinGW 编译器

### 2. 打开项目
- 打开 Qt Creator
- File -> Open File or Project
- 选择 `seckill_client.pro`

### 3. 配置服务器地址
编辑 `NetworkManager.cc`，修改构造函数中的服务器地址：
```cpp
m_serverUrl = "http://你的服务器IP:8080";
```

### 4. 构建运行
- Qt Creator 左下角点击 Run 按钮

## 功能特性

### 主界面 (MainWindow)
- 显示所有秒杀活动列表
- 双击活动进入详情
- 每10秒自动刷新列表
- 实时显示活动状态

### 活动详情页 (ActivityDetailWindow)
- 活动信息展示
- 实时倒计时
- 实时库存显示
- 抢购按钮（倒计时结束后可点击）
- 购买数量选择

## API 接口

客户端调用以下后端 API：

| 接口 | 说明 |
|------|------|
| GET /api/activity/list | 获取活动列表 |
| GET /api/activity/{id} | 获取活动详情 |
| GET /api/activity/{id}/stock | 获取活动库存 |
| GET /api/seckill/countdown/{id} | 获取倒计时 |
| POST /api/seckill/order | 提交抢购订单 |
| GET /api/order/{id} | 查询订单 |
| GET /api/order/status/{id} | 查询订单状态 |

## 注意事项

1. 确保后端服务已启动并运行
2. 服务器地址需要根据实际情况修改
3. user_id 目前硬编码为 1，实际项目中应从登录模块获取
