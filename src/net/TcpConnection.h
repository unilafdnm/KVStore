#pragma once

#include"KVStore.h"

#include"Buffer.h"
#include"CommandParser.h"
#include<functional>
#include<memory>
#include<string>
#include<ostream>


namespace minikv{
std::ostream& operator<<(std::ostream& os,CommandType type);

class EventLoop;
class Channel;
class KVStore;
class ThreadPool;

class TcpConnection:public std::enable_shared_from_this<TcpConnection>
{

public:
    using CloseCallback=std::function<void(int)>;

    TcpConnection(EventLoop* loop,int fd,KVStore& kvstore,ThreadPool* threadPool);
    ~TcpConnection();

    void start();

    void send(const std::string& data);

    int fd()const;
    void setCloseCallback(CloseCallback cb);
    void processInput();
    
private:
   

    void handleRead();
    void handleClose();
    void handleError();
    void handleWrite();

private:
    int _fd;
    EventLoop* _loop;
    std::unique_ptr<Channel> _channel;
    CloseCallback _closeCallback;
    Buffer _inputBuffer;
    Buffer _outputBuffer;
    CommandParser _commandParser;
    ThreadPool* _threadPool;
    KVStore& _kvstore;

};

}