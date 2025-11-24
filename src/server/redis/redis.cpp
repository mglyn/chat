#include "redis.hpp"
#include "redis_pool_singleton.hpp"
#include <muduo/base/Logging.h>
#include <iostream>
using namespace std;

Redis::Redis()
    : _publish_context(nullptr), _subcribe_context(nullptr)
{
}

Redis::~Redis()
{
    // publish 使用池化，归还即可
    if (_publish_context != nullptr)
    {
        RedisPool::instance().release(_publish_context);
        _publish_context = nullptr;
    }
    // subscribe 为长连接，不放入池，直接关闭
    if (_subcribe_context != nullptr)
    {
        redisFree(_subcribe_context);
        _subcribe_context = nullptr;
    }
}

bool Redis::connect()
{
    // 订阅使用独立长期连接（不进入池）
    _subcribe_context = redisConnect("127.0.0.1", 6379);
    if (nullptr == _subcribe_context || _subcribe_context->err)
    {
        LOG_ERROR << "Failed to create subscribe context";
        return false;
    }

    // 在单独的线程中，监听通道上的事件，有消息给业务层进行上报
    thread t([&]() {
        observer_channel_message();
    });
    t.detach();

    LOG_INFO << "connect redis-server success!";
    return true;
}

// 向redis指定的通道channel发布消息
bool Redis::publish(int channel, string message)
{
    // 每次发布时从连接池获取一个短连接，使用完后立即归还，避免单个连接在并发场景下被多个线程复用
    redisContext *ctx = RedisPool::instance().get();
    if (nullptr == ctx)
    {
        LOG_ERROR << "Failed to get publish context from pool";
        return false;
    }

    redisReply *reply = (redisReply *)redisCommand(ctx, "PUBLISH %d %s", channel, message.c_str());
    if (nullptr == reply)
    {
        LOG_ERROR << "publish command failed!";
        RedisPool::instance().release(ctx);
        return false;
    }
    freeReplyObject(reply);
    RedisPool::instance().release(ctx);
    return true;
}

// 向redis指定的通道subscribe订阅消息
bool Redis::subscribe(int channel)
{
    // SUBSCRIBE命令本身会造成线程阻塞等待通道里面发生消息，这里只做订阅通道，不接收通道消息
    // 通道消息的接收专门在observer_channel_message函数中的独立线程中进行
    // 只负责发送命令，不阻塞接收redis server响应消息，否则和notifyMsg线程抢占响应资源
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "SUBSCRIBE %d", channel))
    {
        LOG_ERROR << "subscribe command failed!";
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done))
        {
            LOG_ERROR << "subscribe command failed!";
            return false;
        }
    }
    // redisGetReply

    return true;
}

// 向redis指定的通道unsubscribe取消订阅消息
bool Redis::unsubscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "UNSUBSCRIBE %d", channel))
    {
        LOG_ERROR << "unsubscribe command failed!";
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done))
        {
            LOG_ERROR << "unsubscribe command failed!";
            return false;
        }
    }
    return true;
}

// 在独立线程中接收订阅通道中的消息
void Redis::observer_channel_message()
{
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(this->_subcribe_context, (void **)&reply))
    {
        // 订阅收到的消息是一个带三元素的数组
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            // 给业务层上报通道上发生的消息
            _notify_message_handler(atoi(reply->element[1]->str) , reply->element[2]->str);
        }

        freeReplyObject(reply);
    }

    LOG_ERROR << ">>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<";
}

void Redis::init_notify_handler(function<void(int,string)> fn)
{
    this->_notify_message_handler = fn;
}