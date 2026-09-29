#pragma once
#include"Epoller.h"

#include<unordered_map>

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

private:
    Epoller _epoller;
    bool _running;
    std::unordered_map<int,Channel*> _channels;

};



}