#include"TcpConnection.h"
#include"EventLoop.h"
#include"Channel.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

namespace minikv{


TcpConnection::TcpConnection(EventLoop* loop,int fd)
    :_loop(loop),_fd(fd),_channel(std::make_unique<Channel>(loop,fd))
{
    _channel->setReadCallback(
        [this](){
            handleRead();
        }
    );

    _channel->setCloseCallback(
        [this](){
            handleClose();
        }
    );

    _channel->setErrorCallback(
        [this](){
            handleError();
        }
    );

}
TcpConnection::~TcpConnection(){

    if(_fd>=0){
        close(_fd);
    }

}

void TcpConnection::start(){
    _channel->enableReading();
}

void TcpConnection::send(const std::string& data){
    ::send(_fd,data.data(),data.size(),0);
}

int TcpConnection::fd()const{
    return _fd;
}
void TcpConnection::setCloseCallback(CloseCallback cb){

    _closeCallback=std::move(cb);

}


void TcpConnection::handleRead(){

    char buffer[1024];

    while(true){
        ssize_t n=recv(_fd,buffer,sizeof(buffer),0);
        if(n>0){
            std::string data(buffer,n);
            std::cout<<"recv fd="<<_fd<<": "<<data<<'\n';
            send(data);
        }
        else if(n == 0){
            handleClose();
            break;
        }else{

            if(errno == EAGAIN || errno == EWOULDBLOCK){
                break;
            }
            std::cerr<<"recv failed,fd="<<_fd<<": "<<std::strerror(errno)<<'\n';
            handleClose();
            break;
        }

    }


}
void TcpConnection::handleClose(){

    if(_fd<0){
        return;
    }

    std::cout<<"client disconnection,fd="<<_fd<<'\n';

    _loop->removeChannel(_channel.get());

    int oldFd=_fd;
    close(_fd);
    _fd=-1;
    if(_closeCallback){
        _closeCallback(oldFd);
    }

}
void TcpConnection::handleError(){

    std::cerr<<"connection error,fd="<<_fd<<'\n';
    handleClose();

}



}