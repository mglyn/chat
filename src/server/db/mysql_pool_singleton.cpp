#include "mysql_pool_singleton.hpp"
#include <mutex>
#include <stdexcept>
#include <thread>

namespace {
// 配置参数可根据实际情况调整
const size_t POOL_SIZE = std::thread::hardware_concurrency();
constexpr const char* MYSQL_HOST = "127.0.0.1";
constexpr const char* MYSQL_USER = "root";
constexpr const char* MYSQL_PASS = "259505";
constexpr const char* MYSQL_DB   = "chat";

// 使用局部静态变量保证单例仅被创建一次，且在所有翻译单元中唯一
constexpr int MYSQL_SERVER_PORT = 3306;

MYSQL* create_mysql() {
    MYSQL* conn = mysql_init(nullptr);
    if (!mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DB, MYSQL_SERVER_PORT, nullptr, 0)) {
        // 连接失败直接终止
        LOG_ERROR << "MySQL connect failed";
        throw std::runtime_error("MySQL connect failed");
    }
    mysql_query(conn, "set names gbk");
    LOG_INFO << "MySQLPool: Created connection " << conn;
    return conn;
}

void destroy_mysql(MYSQL* conn) {
    if (conn) {
        LOG_INFO << "MySQLPool: Destroying connection " << conn;
        mysql_close(conn);
    }
}
}

ObjectPool<MYSQL>& MySQLPool::instance() {
    static ObjectPool<MYSQL> pool(POOL_SIZE, create_mysql, destroy_mysql);
    return pool;
}
