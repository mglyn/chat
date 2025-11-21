#pragma once
#include "redis_pool_singleton.hpp"

// RAII 封装，自动归还 Redis 连接
class RedisConnGuard {
public:
    RedisConnGuard() : conn(RedisPool::instance().get()) {}
    ~RedisConnGuard() { if (conn) RedisPool::instance().release(conn); }
    redisContext* get() const { return conn; }
private:
    redisContext* conn;
};