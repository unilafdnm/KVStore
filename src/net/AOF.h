#pragma once

#include<fstream>
#include<mutex>
#include<string>
#include<iostream>
namespace minikv{

class KVStore;

class AOF{
public:
    explicit AOF(const std::string& filename);

    void append(const std::string& command);
    void load(KVStore& kvstore);

private:
    std::mutex _mutex;
    std::ofstream _file;
    std::string _filename;
};

}
