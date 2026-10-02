#pragma once 
#include<thread>
#include<mutex>
#include<deque>
#include<functional>
#include<atomic>
#include<condition_variable>
#include<vector>
namespace minikv{

class ThreadPool{

    using work=std::function<void()>;
public:
    ThreadPool(int maxsize=4);
    ~ThreadPool();
    void submit(work task);
    void workerLoop();
    void stop();

private:
    std::vector<std::thread> _threads;
    std::deque<work> _tasks;
    std::atomic<bool> _stop;
    std::mutex _mutex;
    std::condition_variable _cond;
    int _maxsize;
};



}