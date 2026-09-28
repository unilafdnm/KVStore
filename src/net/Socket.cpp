#include"Socket.h"

#include<fcntl.h>
#include<iostream>

namespace minikv{

    void setNonBlocking(int fd){
        int flags=fcntl(fd,F_GETFL,0);
        if(flags==-1){
            std::cerr<<"fcntl F_GETFL failed\n";
            return;
        }

        if(fcntl(fd,F_SETFL,flags|O_NONBLOCK) ==-1){
            std::cerr<<"fcntl F_SETFL failed\n";
        }

    }

}