#include"CommandExecutor.h"
#include"KVStore.h"
#include"Command.h"

#include<iostream>
#include<thread>

namespace minikv{

CommandExecutor::CommandExecutor(KVStore& kvstore)
    :_kvStore(kvstore)    
{

}
std::string CommandExecutor::execute(const Command& command){
    std::cout<<"execute thread id:"<<std::this_thread::get_id()<<std::endl;

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
    case CommandType::EXPIRE:
       try{
            int64_t seconds=std::stoll(command.value);
            bool result=_kvStore.expire(command.key,seconds);
            return result?"1\r\n":"0\r\n";
       }catch(...){
            return std::string("ERR\r\n");
       }
    case CommandType::TTL:
       {
            int64_t result=_kvStore.ttl(command.key);
            return std::to_string(result)+"\r\n";
       }
       
    default:
        return std::string("ERROR\r\n");
    }


}

}