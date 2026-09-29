#include"Epoller.h"

#include<cerrno>
#include<cstring>
#include<iostream>
#include<unistd.h>

namespace minikv{

Epoller::Epoller(int maxEvents)
    :_epollFd(-1),_events(maxEvents)
{
    _epollFd=epoll_create1(0);
    if(_epollFd<0){
        std::cerr<<"epoll_create failed:"<<std::strerror(errno)<<'\n';        
    }
}
Epoller::~Epoller(){
    
    if(_epollFd>=0){
        close(_epollFd);
    }
    
}

bool Epoller::addFd(int fd,uint32_t events){

    epoll_event event{};
    event.events=events;
    event.data.fd=fd;

    return epoll_ctl(_epollFd,EPOLL_CTL_ADD,fd,&event)==0;

}
bool Epoller::modifyFd(int fd,uint32_t events){
    epoll_event event{};
    event.events=events;
    event.data.fd=fd;

    return epoll_ctl(_epollFd,EPOLL_CTL_MOD,fd,&event)==0;

}
bool Epoller::removeFd(int fd){
    return epoll_ctl(_epollFd,EPOLL_CTL_DEL,fd,nullptr)==0;

}

int Epoller::wait(int timeout=-1){

    return epoll_wait(_epollFd,_events.data(),static_cast<int>(_events.size()),timeout);

}

const epoll_event& Epoller::getEvent(int index)const{
    return _events[index];
}

}