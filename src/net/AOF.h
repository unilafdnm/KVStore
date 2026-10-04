#pragma once


#include<fstream>
#include<mutex>
#include<string>
#include<iostream>
#include<vector>
#include<unordered_map>
namespace minikv{

class KVStore;

class AOF{
public:
    explicit AOF(const std::string& filename);

    void append(const std::string& command);
    void load(KVStore& kvstore);
    bool rewrite(const std::unordered_map<std::string,std::string>& snapshot);
    bool beginRewrite();

private:
    std::mutex _mutex;
    std::ofstream _file;
    std::string _filename;
    bool _rewriting;
    std::vector<std::string> _rewritrBuffer;
};

}
