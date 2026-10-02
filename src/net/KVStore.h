#pragma once

#include<unordered_map>
#include<string>
#include<optional>
#include<mutex>
#include<shared_mutex>
namespace minikv{

class KVStore{
public:
    void set(std::string key,std::string value);
    std::optional<std::string> get(std::string key);
    bool del(std::string key);

private:
    std::unordered_map<std::string,std::string> _kvstore;
    std::shared_mutex _shared_mutex;
};

}