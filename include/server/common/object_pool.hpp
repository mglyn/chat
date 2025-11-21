#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>

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
            _pool.push(_create());
        }
    }

    ~ObjectPool() {
        while (!_pool.empty()) {
            _destroy(_pool.front());
            _pool.pop();
        }
    }

    T* get() {
        std::unique_lock<std::mutex> lock(_mutex);
        while (_pool.empty()) {
            _cond.wait(lock);
        }
        T* obj = _pool.front();
        _pool.pop();
        return obj;
    }

    void release(T* obj) {
        std::lock_guard<std::mutex> lock(_mutex);
        _pool.push(obj);
        _cond.notify_one();
    }

private:
    std::queue<T*> _pool;
    std::mutex _mutex;
    std::condition_variable _cond;
    CreateFunc _create;
    DestroyFunc _destroy;
};
