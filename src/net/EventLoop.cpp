
#include"EventLoop.h"
#include"Channel.h"
#include<cerrno>
#include<iostream>

namespace minikv{


EventLoop::EventLoop()
    :_epoller(1024),_running(false)
{

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


Epoller& EventLoop::getEpoller(){
    return _epoller;
}

}


