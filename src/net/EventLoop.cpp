
#include"EventLoop.h"
#include"Channel.h"
#include"ThreadPool.h"
#include<cerrno>
#include<iostream>
#include<sys/eventfd.h>
#include <unistd.h>
#include <cstdint>
#include <sys/socket.h>

namespace minikv{


EventLoop::EventLoop()
    :_epoller(1024),_running(false)
{

    _wakeupFd=eventfd(0,EFD_CLOEXEC|EFD_NONBLOCK);
    _wakeupChannel=std::make_unique<Channel>(this,_wakeupFd);
    _wakeupChannel->enableReading();
    _wakeupChannel->setReadCallback([this](){
        handleWakeup();
    });
}

void EventLoop::handleWakeup(){
    uint64_t value;
    ::read(_wakeupFd,&value,sizeof(value));
}

void EventLoop::wakeup(){

    uint64_t value=1;
    ::write(_wakeupFd,&value,sizeof(value));

}


void EventLoop::loop(){

    _running=true;

    while(_running){
        int eventCount=_epoller.wait(-1);

        if(eventCount<0){

            if(errno == EINTR){
                continue;
            }

            std::cerr<<"epoll wait failed\n";
            break;
        }
        for(int i=0;i<eventCount;i++){

            const auto& event=_epoller.getEvent(i);
            int fd=event.data.fd;
            
            auto it=_channels.find(fd);
            if(it==_channels.end()){
                continue;
            }

            Channel* channel=it->second;

            channel->setRevents(event.events);
            channel->handleEvent();

        }
        doPendingFunctors();

    }

}

void EventLoop::updateChannel(Channel* channel){

    int fd=channel->fd();

    auto it=_channels.find(fd);
    if(it==_channels.end()){
        _epoller.addFd(fd,channel->events());
        _channels[fd]=channel;
    }

    _epoller.modifyFd(fd,channel->events());

}
void EventLoop::removeChannel(Channel* channel){

    int fd=channel->fd();
    _epoller.removeFd(fd);
    _channels.erase(fd);

}
void EventLoop::queueInLoop(std::function<void()> cb){

    {
        std::lock_guard<std::mutex> lock(_mutex);
        _pendingFunctors.push_back(std::move(cb));
    }
    wakeup();
  

}

void EventLoop::doPendingFunctors(){
    std::unique_lock<std::mutex> lock(_mutex);
    std::vector<std::function<void()>> dst;
    dst.swap(_pendingFunctors);

    lock.unlock();

    for(int i=0;i<dst.size();i++){
        dst[i]();
    }


}


Epoller& EventLoop::getEpoller(){
    return _epoller;
}

}


