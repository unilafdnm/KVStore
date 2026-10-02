#include"TcpConnection.h"
#include"EventLoop.h"
#include"Channel.h"
#include"KVStore.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

namespace minikv{

std::ostream& operator<<(std::ostream& os,CommandType type){
    switch(type)
    {
        case CommandType::GET: os << "GET"; break;
        case CommandType::SET: os << "SET"; break;
        case CommandType::DEL: os << "DEL"; break;
        default: os << "UNKNOWN";
    }
    return os;
    
}


TcpConnection::TcpConnection(EventLoop* loop,int fd,KVStore& kvstore)
    :_loop(loop),_fd(fd),_channel(std::make_unique<Channel>(loop,fd))
    ,_inputBuffer(1024),_kvstore(kvstore)
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

    _channel->setWriteCallback([this](){
        handleWrite();
    });

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

    while(true){
        if(_outputBuffer.readableBytes()==0){
            ssize_t n=::send(_fd,data.data(),data.size(),0);
            if(n>0){

                if(static_cast<std::size_t>(n) == data.size()){
                    break;
                }else if(static_cast<std::size_t>(n) <data.size()){
                    const char* temp=data.data();
                    _outputBuffer.append(temp+n,data.size()-n);
                    break;
                }
            }else if(n==-1){
                if(errno==EAGAIN || errno==EWOULDBLOCK){
                    const char* temp=data.data();
                    _outputBuffer.append(temp,data.size());
                }
                if(errno == EINTR){
                    continue;
                }
                handleError();
                break;
            }else{
                break;
            }
        }else{
            _outputBuffer.append(data.c_str(),data.size());
            break;
        }
    }


    

    if(_outputBuffer.readableBytes()){
        _channel->enableWrite();
    }
    

    
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
        std::optional<Command> result=_commandParser.parse(line);

        if(!result.has_value()){
            send("ERR\r\n");
            continue;
        }
        std::cout<<"complete message TYPE="<<result->type<<" Key="<<result->key<<" Value"<<result->value<<'\n';

        if(result->type==CommandType::GET){
            std::optional<std::string> ret=_kvstore.get(result->key);
            if(ret.has_value()){
                send(ret.value()+"\r\n");
            }else{
                send("get error\r\n");
            }
            
        }else if(result->type==CommandType::SET){
            _kvstore.set(result->key,result->value);
            send("Set success\r\n");
        }else if(result->type==CommandType::DEL){
            _kvstore.del(result->key);
            send("DEL success\r\n");
        }



      
        
    }

  

}


void TcpConnection::handleRead(){

    char temp[4096];

    while(true){
        ssize_t n=recv(_fd,temp,sizeof(temp),0);
        if(n>0){
            _inputBuffer.append(temp,static_cast<std::size_t>(n));
        }
        else if(n == 0){
            processInput();
            handleClose();
            return;
        }else{

            if(errno == EAGAIN || errno == EWOULDBLOCK){
                break;
            }
            if(errno == EINTR){
                continue;
            }
            std::cerr<<"recv failed,fd="<<_fd<<": "<<std::strerror(errno)<<'\n';
            handleError();
            return;
        }
        
    }

    processInput();

}

void TcpConnection::handleWrite(){

    if(_fd < 0 || _outputBuffer.readableBytes() == 0){
        return;
    }


    while(true){
        ssize_t n=::send(_fd,_outputBuffer.peek(),_outputBuffer.readableBytes(),0);
        if(n>0){
            _outputBuffer.retrieve(n);
            if(_outputBuffer.readableBytes()==0){
                _channel->disableWrite();
                break;
            }
        }else if(n == -1){
            if(errno == EAGAIN || errno==EWOULDBLOCK){
                break;
            }
            if(errno == EINTR){
                continue;
            }
           handleError();
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