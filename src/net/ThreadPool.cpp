#include"ThreadPool.h"

#include<iostream>

namespace minikv{

ThreadPool::ThreadPool(int maxsize)
    :_stop(false),_maxsize(maxsize)
{

    for(int i=0;i<maxsize;i++){
        _threads.emplace_back([this,&i](){
            std::cout<<"thread id="<<i<<std::endl;
            workerLoop();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        });
    }

}
ThreadPool::~ThreadPool(){
    stop();
    for(int i=0;i<_maxsize;i++){
        if(_threads[i].joinable()){
            _threads[i].join();
        }
    }

}
void ThreadPool::stop(){
    _stop=true;
     _cond.notify_all();
}
void ThreadPool::submit(work task){

    std::lock_guard<std::mutex> lock(_mutex);
    if(_stop){
        return;
    }
    _tasks.push_back(std::move(task));
    _cond.notify_one();

}
void ThreadPool::workerLoop(){

    while(true){
        std::unique_lock<std::mutex> lock(_mutex);
        _cond.wait(lock,[this](){
            return _stop || !_tasks.empty();
        });

        if(_stop && _tasks.empty()){
            return;
        }

        auto task=std::move(_tasks.front());
        _tasks.pop_front();

        lock.unlock();

        task();
    }

}


}