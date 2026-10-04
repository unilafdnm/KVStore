#pragma once

#include<unordered_map>
#include<string>
#include<optional>
#include<mutex>
#include<shared_mutex>
#include<atomic>
#include<thread>

namespace minikv{


class AOF;

class KVStore{
public:
    KVStore(AOF* aof);
    ~KVStore();
    void set(std::string key,std::string value,bool writeAof=true);
    std::optional<std::string> get(std::string key);
    bool del(std::string key,bool writeAof=true);
    std::unordered_map<std::string,std::string> snapshot();
    bool rewriteAOF();
    bool expire(const std::string&key,int64_t seconds,bool writeAof=true);
    bool expireAt(const std::string& key,int64_t timestampMs,bool writeAof=true);
    int64_t ttl(const std::string& key);
private:
    static int64_t nowMs();
    void cleanupExpired();


private:
    std::unordered_map<std::string,std::string> _kvstore;
    std::shared_mutex _shared_mutex;
    AOF* _aof;
    std::unordered_map<std::string,int64_t> _expires;

    std::atomic<bool> _stopCleaner{false};
    std::thread _cleanerThread;


};

}