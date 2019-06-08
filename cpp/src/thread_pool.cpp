#include "thread_pool.h"
//#include <iostream>

ThreadPool::ThreadPool(): worker_count(4) {
}

ThreadPool::ThreadPool(std::size_t count): worker_count(count) {
}

void ThreadPool::init() {
    this->workers.reserve(this->worker_count);
    for (std::size_t i = 0; i < this->worker_count; ++i) {
        auto sp = shared_from_this();
        //std::cout << "begin declare thread func" << std::endl; 
        //auto t_func = [](std::shared_ptr<ThreadPool> sp) {
        auto t_func = [sp]() {
            //std::cout << "this is thread " << std::this_thread::get_id() 
            //    << ", share count : " << sp.use_count() << std::endl; 
            while (true) {
                //std::this_thread::sleep_for(std::chrono::seconds(2));
                //std::function<void()> func = nullptr;
                //Func f = sp->get_func();
                std::unique_lock<std::mutex> lock(sp->lock);
                if (sp->funcs.empty()) {
                    //lock.unlock();
                    //std::cout << "this is thread " << std::this_thread::get_id() << " call get_func but empty " << std::endl;
                    sp->cv.wait(lock, [&] { return sp->events_size() > 0; });
                }

                Func f = std::move(sp->funcs.front());
                sp->funcs.pop_front();
                lock.unlock();

                //Func f ＝ nullptr;
                if (f == nullptr) {
                    //std::cout << "this is thread " << std::this_thread::get_id() 
                    //    << ", share count : " << sp.use_count()  << " func nullptr : " << std::endl; 
                    continue;
                }

                if (nullptr != f) {
                    f();
                }

                std::this_thread::yield();
            }
        };
        //std::cout << "after declare thread func" << std::endl; 
        std::thread t = std::thread(t_func);
        t.detach();
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        this->workers.push_back(std::move(t));
    }

}

ThreadPool::~ThreadPool() {
}

void ThreadPool::post_func(Func f) {
    if (nullptr == f) {
        return ;
    }

    {
        std::lock_guard<std::mutex> lock(this->lock);
        this->funcs.push_back(f);
    }  //lock.unlock();

    this->cv.notify_one();
}