
add_library(net
    Socket.cpp
    Epoller.cpp
    EventLoop.cpp
    Channel.cpp
    TcpConnection.cpp
    Buffer.cpp
    CommandParser.cpp
    KVStore.cpp
)


target_include_directories(net 
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)