/// class thread pool 
/// design: 
///  a list with a mutex lock and a thread condition variable to gerantee 
/// thread safe while insert function for back 
/// thread execute multi work thread get a function object from list while running, 
/// if get a function failed
/// then waiting for 
#pragma once

#include <memory>
#include <list>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

typedef std::function<void()> Func;

// todo: 开始的时候统一分配事件，不再由线程一个一个的取，
class ThreadEventsContainer {

public:
    std::mutex lock;
    std::condition_variable cv;
    std::list<Func> funcs;
};

class ThreadPool: public std::enable_shared_from_this<ThreadPool> {
public:
    ThreadPool();
    ThreadPool(std::size_t count);
    ~ThreadPool();

    void init();
    void post_func(Func f); 

    void cond_wait();

    std::size_t events_size() {
        std::lock_guard<std::mutex> lock(this->lock);
        return this->funcs.size();
    }

    void append_events(std::list<Func> fs) {
        std::lock_guard<std::mutex> lock(this->lock);
        for (auto it : fs) {
            this->funcs.emplace_back(it);
        }
    }

private:
    Func get_func(); 

private:
    std::size_t worker_count;
    std::vector<std::thread> workers;

    std::vector<ThreadEventsContainer> events;
};