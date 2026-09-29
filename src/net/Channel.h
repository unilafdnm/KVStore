#pragma

#include<cstdint>
#include<functional>

namespace minikv{

class EventLoop;

class Channel{

public:
    using EventCallback=std::function<void()>;

    Channel(EventLoop* loop,int fd);
    int fd()const;

    uint32_t events()const;

    void setEvents(uint32_t events);

    void setRevents(uint32_t revents);
    void setReadCallback(EventCallback cb);
    void setWriteCallback(EventCallback cb);
    void setCloseCallback(EventCallback cb);
    void setErrorCallback(EventCallback cb);

    void enableReading();
    void enableWrite();
    void disableWrite();
    void disableAll();

    void handleEvent();

private:
    EventLoop* _loop;
    int _fd;

    uint32_t _events;
    uint32_t _revents;


    EventCallback _readCallback;
    EventCallback _writeCallback;
    EventCallback _closeCallback;
    EventCallback _errorCallback;

};

}