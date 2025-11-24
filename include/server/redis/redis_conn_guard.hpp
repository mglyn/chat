#pragma once
#include "redis_pool_singleton.hpp"
#include <muduo/base/Logging.h>  // 使用 muduo Logger

// RAII 封装，自动归还 Redis 连接
class RedisConnGuard {
public:
    RedisConnGuard() : _conn(RedisPool::instance().get()) {}

    // 禁止拷贝，防止双重归还
    RedisConnGuard(const RedisConnGuard&) = delete;
    RedisConnGuard& operator=(const RedisConnGuard&) = delete;

    // 允许移动
    RedisConnGuard(RedisConnGuard&& other) noexcept : _conn(other._conn) {
        other._conn = nullptr;
    }
    RedisConnGuard& operator=(RedisConnGuard&& other) noexcept {
        if (this != &other) {
            release();
            _conn = other._conn;
            other._conn = nullptr;
        }
        return *this;
    }

    redisContext* get() const { return _conn; }
    ~RedisConnGuard() { release(); }

private:
    void release() {
        if (_conn) {
            RedisPool::instance().release(_conn);
            _conn = nullptr;
        }
    }
private:
    redisContext* _conn{nullptr};
};