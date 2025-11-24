# Chat 服务（C++）

这是一个基于 muduo 网络库、hiredis、MySQL 的轻量级即时通讯（Chat）服务示例，包含服务端和客户端可执行文件（`ChatServer` 和 `ChatClient`）。项目使用 CMake 构建。

## 主要特性

- 基于 muduo 的高性能网络框架（TCP/多线程）
- 使用 MySQL 存储用户、好友、群组、离线消息等数据
- 使用 Redis 发布/订阅机制实现跨进程/跨服务器的消息分发
- 连接池模式管理 MySQL/Redis 连接
- json 消息格式（使用 single-header `json.hpp`）

## 仓库结构（核心文件/目录）

- `src/` - 源代码
  - `client/` - 客户端程序
  - `server/` - 服务端程序（包含 chatServer、chatService、db、redis 等模块）
- `include/` - 头文件
- `thirdparty/json.hpp` - nlohmann::json 单文件头
- `CMakeLists.txt` - 顶层 CMake
- `bin/` - 构建输出（可执行文件）

## 依赖

在开始前，请确保系统安装了下列依赖：

- CMake >= 3.10
- 一个支持 C++11/C++14 的编译器（g++/clang）
- muduo 库（用于网络与线程）
- hiredis（Redis C 客户端）
- MySQL 客户端开发库（libmysqlclient）
- pthreads（通常由系统提供）

在 Debian/Ubuntu 上，可通过以下命令安装常见依赖（示例）：

```bash
sudo apt update
sudo apt install build-essential cmake libmysqlclient-dev libhiredis-dev libpthread-stubs0-dev
# muduo 需要单独编译/安装，或把 muduo 源码纳入项目构建
```

注意：muduo 并不总是存在于系统包管理器中，建议提前编译并安装 muduo，或把 muduo 编译输出的 include/lib 路径添加到系统路径或 CMake 配置中。

## 构建

1. 创建构建目录并进入：

```bash
mkdir -p build
cd build
```

2. 运行 CMake 并构建：

```bash
cmake ..
make -j$(nproc)
```

构建完成后，可执行文件会输出到项目根目录下的 `bin/` 目录（由顶层 `CMakeLists.txt` 指定）：
- `bin/ChatServer`
- `bin/ChatClient`

## 运行

服务端启动示例：

```bash
# 在项目根或 bin 目录下运行
./bin/ChatServer 0.0.0.0 6007 > server.log 2>&1 &
```

参数：
- 第1个参数：监听 IP（例如 0.0.0.0 或 127.0.0.1）
- 第2个参数：监听端口（例如 6007）

客户端运行方式请参照 `src/client/main.cpp`（会解析命令行参数并与服务端建立连接）。

## 配置（MySQL / Redis）

代码中默认使用以下配置（可在源码对应文件中调整）：

- MySQL（在 `src/server/db/mysql_pool_singleton.cpp`）：
  - host: 127.0.0.1
  - port: 3306
  - user: root
  - password: 259505
  - database: chat
  - 字符集：GBK（代码中通过 `set names gbk` 设置）

- Redis（在 `src/server/redis` 模块）：
  - host: 127.0.0.1
  - port: 6379
  - 使用 Redis 连接池（publish 操作从池里临时取连接）
  - subscribe 使用独立长期连接用于接收消息

请确保 MySQL/Redis 服务已按上述配置启动，或修改源码以匹配你的环境。
