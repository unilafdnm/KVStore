#pragma once
#include<string>



namespace minikv{
enum class CommandType{
    GET,
    SET,
    DEL,
    EXPIRE,
    TTL,
    EXPIREAT
};

struct Command{
    CommandType type;
    std::string key;
    std::string value;
};


}