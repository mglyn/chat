#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <unordered_set>
#include <muduo/base/Logging.h>  // 使用 muduo Logger

// 通用对象池模板，适用于任何指针类型
// T: 连接类型（如 MYSQL*、redisContext*）
template <typename T>
class ObjectPool {
public:
    using CreateFunc = std::function<T*()>;
    using DestroyFunc = std::function<void(T*)>;

    ObjectPool(size_t poolSize, CreateFunc create, DestroyFunc destroy)
        : _create(create), _destroy(destroy)
    {
        for (size_t i = 0; i < poolSize; ++i) {
            T* obj = _create();
            if (obj) {
                _pool.push(obj);
                _inPool.insert(obj);
                LOG_INFO << "ObjectPool: Created object " << obj;
            }
        }
    }

    ~ObjectPool() {
        while (!_pool.empty()) {
            T* obj = _pool.front();
            _pool.pop();
            if (obj) {
                LOG_INFO << "ObjectPool: Destroying object " << obj;
                _destroy(obj);
            }
        }
        _inPool.clear();
    }

    T* get() {
        std::unique_lock<std::mutex> lock(_mutex);
        while (_pool.empty()) {
            LOG_INFO << "ObjectPool: Pool empty, waiting...";
            _cond.wait(lock);
        }
        T* obj = _pool.front();
        _pool.pop();
        _inPool.erase(obj);
        return obj;
    }

    void release(T* obj) {
        if (!obj) {
            LOG_ERROR << "ObjectPool: Attempt to release null object!";
            return;
        }
        std::lock_guard<std::mutex> lock(_mutex);
        if (_inPool.count(obj)) {
            LOG_ERROR << "ObjectPool: Duplicate release detected for object " << obj;
            return; // 防止重复入队造成二次释放
        }
        _pool.push(obj);
        _inPool.insert(obj);
        _cond.notify_one();
    }

private:
    std::queue<T*> _pool;
    std::unordered_set<T*> _inPool; // 追踪当前在池中的对象，检测重复归还
    std::mutex _mutex;
    std::condition_variable _cond;
    CreateFunc _create;
    DestroyFunc _destroy;
};
