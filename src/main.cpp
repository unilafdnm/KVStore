#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>


int main(){

    constexpr int PORT=8888;
    constexpr int MAX_EVENTS=1024;
    constexpr int BUFFER_SIZE=1024;

    int listenFd=socket(AF_INET,SOCK_STREAM,0);

    std::cout<<listen<<std::endl;

    if(listenFd <0){
        std::cerr<<"socket failedn\n";
        return -1;
    }

    int opt=1;
    setsockopt(listenFd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    minikv::setNonBlocking(listenFd);

    sockaddr_in serverAddr{};
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_addr.s_addr=INADDR_ANY;
    serverAddr.sin_port=htons(PORT);
    if(bind(listenFd,reinterpret_cast<sockaddr*>(&serverAddr),sizeof(serverAddr)) < 0){

        std::cerr<<"bind failed:"<<std::strerror(errno)<<'\n';
        close(listenFd);
        return 1;

    }

    listen(listenFd,128);

    minikv::EventLoop loop;

    std::unordered_map<int,std::unique_ptr<minikv::Channel>> clientChannels;

    minikv::Channel listenChannel(&loop,listenFd);

    listenChannel.setReadCallback(
        [&](){
            while(true){

                sockaddr_in clientAddr{};
                socklen_t clientLen=sizeof(clientAddr);

                int clientFd=accept(listenFd,reinterpret_cast<sockaddr*>(&clientAddr),&clientLen);

                if(clientFd<0){
                    if (errno == EAGAIN ||errno == EWOULDBLOCK) {
                        break;
                    }
                    std::cerr<< "accept failed\n";
                    break;
                }

                minikv::setNonBlocking(clientFd);

                auto channel=std::make_unique<minikv::Channel>(&loop,clientFd);
                channel->setReadCallback(
                    [&](){

                        char buffer[BUFFER_SIZE];

                        while (true) {

                            ssize_t n = recv(clientFd,buffer,sizeof(buffer),0);

                            if (n > 0) {
                                std::cout<< "recv fd="<< clientFd<< ": "<< std::string(buffer,n)<< '\n';

                                send(clientFd,buffer,n,0);
                            }

                            else if (n == 0) {

                                std::cout<< "client disconnected, fd="<< clientFd<< '\n';

                                auto it =clientChannels.find(clientFd);

                                if (it !=clientChannels.end()) {
                                    loop.removeChannel(it->second.get());
                                    clientChannels.erase(it);
                                }

                                close(clientFd);

                                break;
                            }

                            else {

                                if (errno == EAGAIN ||errno == EWOULDBLOCK) {
                                    break;
                                }

                                auto it =clientChannels.find(clientFd);

                                if (it !=clientChannels.end()) {

                                    loop.removeChannel(it->second.get());
                                    clientChannels.erase(it);
                                }

                                close(clientFd);

                                break;
                            }
                        }


                    }
                );

            }
        }
    );

    listenChannel.enableReading();
    loop.loop();
    close(listenFd);

    return 0;
}