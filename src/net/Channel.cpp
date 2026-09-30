#include"Channel.h"
#include"EventLoop.h"


namespace minikv{

Channel::Channel(EventLoop* loop,int fd)
    :_loop(loop),_fd(fd),_events(0),_revents(0)
{

}
int Channel::fd()const{
    return _fd;
}

uint32_t Channel::events()const{
    return _events;
}

void Channel::setEvents(uint32_t events){
    _events=events;
}

void Channel::setRevents(uint32_t revents){
    _revents=revents;
}
void Channel::setReadCallback(EventCallback cb){
    _readCallback=std::move(cb);
}
void Channel::setWriteCallback(EventCallback cb){
    _writeCallback=std::move(cb);
}
void Channel::setCloseCallback(EventCallback cb){
    _closeCallback=std::move(cb);
}
void Channel::setErrorCallback(EventCallback cb){
    _errorCallback=std::move(cb);
}

void Channel::enableReading(){
    _events |=EPOLLIN;
    _loop->updateChannel(this);
}
void Channel::enableWrite(){
    _events|=EPOLLOUT;
    _loop->updateChannel(this);
}
void Channel::disableWrite(){
    _events&=~EPOLLOUT;
    _loop->updateChannel(this);
}
void Channel::disableAll(){
    _events=0;
    _loop->updateChannel(this);
}

void Channel::handleEvent(){

    if(_fd<0){
        return;
    }

    if(_revents&EPOLLIN){
        if(_readCallback){
            _readCallback();
        }
    }
    if(_revents&EPOLLOUT){
        if(_writeCallback){
            _writeCallback();
        }
    }

    if(_revents&EPOLLERR){
        if(_errorCallback){
            _errorCallback();
        }
    }

    if(_revents&EPOLLHUP){
        if(_closeCallback){
            _closeCallback();
        }
    }


}

}