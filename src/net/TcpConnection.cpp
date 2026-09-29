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
    ,_inputBuffer(1024)
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

void TcpConnection::processInput(){

    while(true){
        const char* crlf=_inputBuffer.findCRLF();

        if(!crlf){
            break;
        }
        std::size_t len=crlf-_inputBuffer.peek();
        std::string line=_inputBuffer.retrieveAsString(len);

        _inputBuffer.retrieve(2);  
        std::cout<<"complete message:"<<line<<'\n';

        send(line+"\r\n");
    }

  

}


void TcpConnection::handleRead(){

    char temp[4096];

    while(true){
        ssize_t n=recv(_fd,temp,sizeof(temp),0);
        if(n>0){
            _inputBuffer.append(temp,static_cast<std::size_t>(n));
            std::cout<<"1"<<std::endl;
        }
        else if(n == 0){
            processInput();

            handleClose();
            break;
        }else{

            if(errno == EAGAIN || errno == EWOULDBLOCK){
                std::cout<<"3"<<std::endl;
                break;
            }
            std::cerr<<"recv failed,fd="<<_fd<<": "<<std::strerror(errno)<<'\n';
            handleClose();
            std::cout<<"4"<<std::endl;
            break;
        }
        
    }

    processInput();

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