#pragma once
#include"Epoller.h"

#include<unordered_map>
#include<functional>
#include<mutex>
#include<memory>
namespace minikv{

class Channel;

class EventLoop
{

public:
    EventLoop();
    void loop();

    Epoller& getEpoller();

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    void handleWakeup();
    void queueInLoop(std::function<void()> cb);

private:
   
    void doPendingFunctors();
    void wakeup();

private:
    Epoller _epoller;
    bool _running;
    std::unordered_map<int,Channel*> _channels;
    std::mutex _mutex;
    std::vector<std::function<void()>> _pendingFunctors;
    int _wakeupFd;
    std::unique_ptr<Channel> _wakeupChannel;

};



}