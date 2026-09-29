#pragma once

#include"Buffer.h"

#include<functional>
#include<memory>
#include<string>

namespace minikv{

class EventLoop;
class Channel;

class TcpConnection{

public:
    using CloseCallback=std::function<void(int)>;

    TcpConnection(EventLoop* loop,int fd);
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

private:
    int _fd;
    EventLoop* _loop;
    std::unique_ptr<Channel> _channel;
    CloseCallback _closeCallback;
    Buffer _inputBuffer;
};

}