#pragma once
#include <hiredis/hiredis.h>
#include <string>
#include "server/common/object_pool.hpp"

// Redis连接池全局单例
class RedisPool {
public:
    static ObjectPool<redisContext>& instance();
};