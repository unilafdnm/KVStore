#pragma once
#include<string>



namespace minikv{
enum class CommandType{
    GET,
    SET,
    DEL
};

struct Command{
    CommandType type;
    std::string key;
    std::string value;
};


}