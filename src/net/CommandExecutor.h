#pragma once

#include<string>

namespace minikv{

class KVStore;
class Command;

class CommandExecutor{

public:
    CommandExecutor(KVStore& kvstore);
    std::string execute(const Command& command);

private:
    KVStore& _kvStore;
};

}