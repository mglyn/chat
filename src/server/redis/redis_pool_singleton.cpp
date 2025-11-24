#include "redis_pool_singleton.hpp"
#include <mutex>
#include <stdexcept>
#include <thread>

namespace {
// 配置参数可根据实际情况调整
const size_t POOL_SIZE = std::thread::hardware_concurrency();
constexpr const char* REDIS_HOST = "127.0.0.1";
constexpr int REDIS_PORT = 6379;

ObjectPool<redisContext>* g_pool = nullptr;
std::once_flag g_flag;

redisContext* create_redis() {
    redisContext* ctx = redisConnect(REDIS_HOST, REDIS_PORT);
    if (!ctx || ctx->err) {
        LOG_ERROR << "Redis connect failed";
        throw std::runtime_error("Redis connect failed");
    }
    LOG_INFO << "RedisPool: Created connection " << ctx;
    return ctx;
}

void destroy_redis(redisContext* ctx) {
    if (ctx) {
        LOG_INFO << "RedisPool: Destroying connection " << ctx;
        redisFree(ctx);
    }
}
}

ObjectPool<redisContext>& RedisPool::instance() {
    static ObjectPool<redisContext> g_pool(POOL_SIZE, create_redis, destroy_redis);
    return g_pool;
}