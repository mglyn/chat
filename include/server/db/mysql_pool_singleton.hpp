#pragma once
#include <mysql/mysql.h>
#include <string>
#include "server/common/object_pool.hpp"

// MySQL连接池全局单例
class MySQLPool {
public:
    static ObjectPool<MYSQL>& instance();
};
