#include"CommandExecutor.h"
#include"KVStore.h"
#include"Command.h"


namespace minikv{

CommandExecutor::CommandExecutor(KVStore& kvstore)
    :_kvStore(kvstore)    
{

}
std::string CommandExecutor::execute(Command& command){

    switch (command.type)
    {
    case CommandType::SET:
        _kvStore.set(command.key,command.value);
        return std::string("OK\r\n");
    case CommandType::GET:
    {
        std::optional<std::string> result=_kvStore.get(command.key);
        if(result.has_value()){
            return std::string(result.value()+"\r\n");
        }
        else{
            return std::string("NIL\r\n");
        }
    }
       
    case CommandType::DEL:
        return _kvStore.del(command.key)?std::string("1\r\n"):std::string("0\r\n");
    default:
        return std::string("ERROR\r\n");
    }


}

}