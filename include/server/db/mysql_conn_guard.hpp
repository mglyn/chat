#pragma once
#include "mysql_pool_singleton.hpp"
#include <muduo/base/Logging.h>  // 使用 muduo Logger

// RAII 封装，自动归还 MySQL 连接
class MySQLConnGuard {
public:
    MySQLConnGuard() : _conn(MySQLPool::instance().get()) {}

    // 禁止拷贝，防止双重归还
    MySQLConnGuard(const MySQLConnGuard&) = delete;
    MySQLConnGuard& operator=(const MySQLConnGuard&) = delete;

    // 允许移动
    MySQLConnGuard(MySQLConnGuard&& other) noexcept : _conn(other._conn) {
        other._conn = nullptr;
    }
    MySQLConnGuard& operator=(MySQLConnGuard&& other) noexcept {
        if (this != &other) {
            release();
            _conn = other._conn;
            other._conn = nullptr;
        }
        return *this;
    }

    MYSQL* get() const { return _conn; }

    ~MySQLConnGuard() { release(); }

private:
    void release() {
        if (_conn) {
            MySQLPool::instance().release(_conn);
            _conn = nullptr;
        }
    }
private:
    MYSQL* _conn{nullptr};
};
