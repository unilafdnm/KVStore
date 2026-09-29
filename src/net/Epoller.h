#pragma once

#include<sys/epoll.h>
#include<vector>

namespace minikv{


class Epoller{

public:
    explicit Epoller(int maxEvents=1024);
    ~Epoller();

    bool addFd(int fd,uint32_t events);
    bool modifyFd(int fd,uint32_t events);
    bool removeFd(int fd);

    int wait(int timeout=-1);

    const epoll_event& getEvent(int index)const;

private:
    int _epollFd;
    std::vector<epoll_event> _events;
};


}