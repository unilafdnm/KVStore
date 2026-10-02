#pragma once
#include"Command.h"
#include<optional>

namespace minikv{

class CommandParser{

public:
    CommandParser()=default;
    std::optional<minikv::Command> parse(const std::string& line);

};


}