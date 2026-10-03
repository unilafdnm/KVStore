#include"Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "TcpConnection.h"
#include"ThreadPool.h"
#include"KVStore.h"
#include"AOF.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>

int main()
{
    minikv::AOF aof("appendonly.aof");
    minikv::KVStore kvtore(&aof);

    aof.load(kvtore);

    constexpr int PORT = 8888;
    //std::shared_ptr<minikv::ThreadPool> threadpool=std::make_shared<minikv::ThreadPool>();
    minikv::ThreadPool threadPool{};
    int listenFd = socket(AF_INET,SOCK_STREAM,0);

    if (listenFd < 0) {
        std::cerr << "socket failed\n";
        return 1;
    }

    int opt = 1;

    setsockopt(listenFd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    minikv::setNonBlocking(listenFd);

    sockaddr_in serverAddr{};

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr =INADDR_ANY;
    serverAddr.sin_port =htons(PORT);

    if (bind(listenFd,reinterpret_cast<sockaddr*>(&serverAddr),sizeof(serverAddr)) < 0) {

        std::cerr<< "bind failed: "<< std::strerror(errno)<< '\n';
        close(listenFd);
        return 1;
    }

    if (listen(listenFd,128) < 0) {
        std::cerr<< "listen failed\n";
        close(listenFd);
        return 1;
    }

    std::cout<< "MiniKV listening on port "<< PORT<< '\n';
    minikv::EventLoop loop;
    std::unordered_map<int,std::shared_ptr<minikv::TcpConnection>> connections;

    minikv::Channel listenChannel(&loop,listenFd);

    listenChannel.setReadCallback(
        [&]() {
            while (true) {

                sockaddr_in clientAddr{};

                socklen_t clientLen =sizeof(clientAddr);

                int clientFd = accept(listenFd,reinterpret_cast<sockaddr*>(&clientAddr),&clientLen);

                if (clientFd < 0) {
                    if (errno == EAGAIN ||errno ==EWOULDBLOCK) {
                        break;
                    }

                    std::cerr<< "accept failed\n";
                    break;
                }

                minikv::setNonBlocking(clientFd);

                std::cout<< "client connected, fd="<< clientFd<< '\n';

                auto connection =std::make_shared<minikv::TcpConnection>(&loop,clientFd,kvtore,&threadPool);

                connection->setCloseCallback(
                        [&](int fd) {
                            loop.queueInLoop([&,fd](){
                                 connections.erase(fd);
                            });
                        }
                );

                connections[clientFd] = connection;

                connection->start();
            }
        }
    );

    listenChannel.enableReading();

    loop.loop();

    close(listenFd);

    return 0;
}