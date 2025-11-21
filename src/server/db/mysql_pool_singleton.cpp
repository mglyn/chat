#include "mysql_pool_singleton.hpp"
#include <mutex>
#include <stdexcept>

namespace {
// 配置参数可根据实际情况调整
constexpr size_t POOL_SIZE = 10;
constexpr const char* MYSQL_HOST = "127.0.0.1";
constexpr const char* MYSQL_USER = "root";
constexpr const char* MYSQL_PASS = "259505";
constexpr const char* MYSQL_DB   = "chat";

ObjectPool<MYSQL>* g_pool = nullptr;
std::once_flag g_flag;

MYSQL* create_mysql() {
    MYSQL* conn = mysql_init(nullptr);
    if (!mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DB, MYSQL_PORT, nullptr, 0)) {
        // 连接失败直接终止
        throw std::runtime_error("MySQL connect failed");
    }
    mysql_query(conn, "set names gbk");
    return conn;
}

void destroy_mysql(MYSQL* conn) {
    if (conn) mysql_close(conn);
}
}

ObjectPool<MYSQL>& MySQLPool::instance() {
    std::call_once(g_flag, []() {
        g_pool = new ObjectPool<MYSQL>(POOL_SIZE, create_mysql, destroy_mysql);
    });
    return *g_pool;
}
