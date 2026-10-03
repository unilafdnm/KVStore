#pragma once

#include<unordered_map>
#include<string>
#include<optional>
#include<mutex>
#include<shared_mutex>

namespace minikv{


class AOF;

class KVStore{
public:
    KVStore(AOF* aof);
    void set(std::string key,std::string value,bool writeAof=true);
    std::optional<std::string> get(std::string key);
    bool del(std::string key,bool writeAof=true);

private:
    std::unordered_map<std::string,std::string> _kvstore;
    std::shared_mutex _shared_mutex;
    AOF* _aof;
};

}