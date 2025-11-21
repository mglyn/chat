#include "redis_pool_singleton.hpp"
#include <mutex>
#include <stdexcept>

namespace {
// 配置参数可根据实际情况调整
constexpr size_t POOL_SIZE = 10;
constexpr const char* REDIS_HOST = "127.0.0.1";
constexpr int REDIS_PORT = 6379;

ObjectPool<redisContext>* g_pool = nullptr;
std::once_flag g_flag;

redisContext* create_redis() {
    redisContext* ctx = redisConnect(REDIS_HOST, REDIS_PORT);
    if (!ctx || ctx->err) {
        throw std::runtime_error("Redis connect failed");
    }
    return ctx;
}

void destroy_redis(redisContext* ctx) {
    if (ctx) redisFree(ctx);
}
}

ObjectPool<redisContext>& RedisPool::instance() {
    std::call_once(g_flag, []() {
        g_pool = new ObjectPool<redisContext>(POOL_SIZE, create_redis, destroy_redis);
    });
    return *g_pool;
}