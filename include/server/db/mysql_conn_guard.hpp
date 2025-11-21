#pragma once
#include "mysql_pool_singleton.hpp"

// RAII 封装，自动归还 MySQL 连接
class MySQLConnGuard {
public:
    MySQLConnGuard() : conn(MySQLPool::instance().get()) {}
    ~MySQLConnGuard() { if (conn) MySQLPool::instance().release(conn); }
    MYSQL* get() const { return conn; }
private:
    MYSQL* conn;
};
