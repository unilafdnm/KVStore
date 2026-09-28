#include<sys/epoll.h>
#include<arpa/inet.h>
#include<cerrno>
#include<cstring>
#include<fcntl.h>
#include<iostream>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>

void setNonBlockint(int fd){
    int flags=fcntl(fd,F_GETFL,0);

    fcntl(fd,F_SETFL,flags|O_NONBLOCK);

}

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
    setNonBlockint(listenFd);
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

    int epollFd=epoll_create1(0);

    epoll_event listenEvent{};
    listenEvent.events=EPOLLIN;
    listenEvent.data.fd=listenFd;

    epoll_ctl(epollFd,EPOLL_CTL_ADD,listenFd,&listenEvent);

    epoll_event events[MAX_EVENTS];

    while(true){

        int eventCount=epoll_wait(epollFd,events,MAX_EVENTS,-1);



        for(int i=0;i<eventCount;i++){

            if(events[i].data.fd==listenFd){

                while(true){
                    sockaddr_in clientAddr{};
                    socklen_t clientLent=sizeof(clientAddr);

                    int clientFd=accept(listenFd,reinterpret_cast<sockaddr*>(&clientAddr),&clientLent);
                    if(clientFd<0){
                        if(errno == EAGAIN || errno == EWOULDBLOCK){
                            break;
                        }

                        break;
                    }
                    
                    setNonBlockint(clientFd);

                    epoll_event clientEvent{};
                    clientEvent.events=EPOLLIN;
                    clientEvent.data.fd=clientFd;
                    epoll_ctl(epollFd,EPOLL_CTL_ADD,clientFd,&clientEvent);
                }
               
            }else{
                char buffer[BUFFER_SIZE];

                while(true){
                    ssize_t n=recv(events[i].data.fd,buffer,sizeof(buffer),0);
                    if(n>0){
                        std::cout<<"recv fd="<<events[i].data.fd<<" : "<<std::string(buffer,n)<<'\n';
                        send(events[i].data.fd,buffer,n,0);
                    }
                    else if(n==0){
                        epoll_ctl(epollFd,EPOLL_CTL_DEL,events[i].data.fd,nullptr);
                        close(events[i].data.fd);
                        break;
                    }else{
                        if(errno == EAGAIN || errno == EWOULDBLOCK){
                            break;
                        }
                        std::cerr<< "recv failed, fd : "<< std::strerror(errno)<< '\n';

                        epoll_ctl(epollFd,EPOLL_CTL_DEL,events[i].data.fd,nullptr);
                        close(events[i].data.fd);
                        break;

                    }
                }

            }

        }


    }
    close(epollFd);
    close(listenFd);

    return 0;
}