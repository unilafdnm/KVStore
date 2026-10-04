#include"KVStore.h"
#include"AOF.h"
#include<chrono>
#include<thread>
namespace minikv
{

KVStore::KVStore(AOF* aof)
    :_aof(aof)
{

    _cleanerThread=std::thread([this](){

        while(!_stopCleaner){
            std::this_thread::sleep_for(std::chrono::seconds(1));

            if(!_stopCleaner){
                break;
            }
            cleanupExpired();
        }
    });

}

KVStore::~KVStore(){
    _stopCleaner=true;

    if(_cleanerThread.joinable()){
        _cleanerThread.join();
    }

}

void KVStore::cleanupExpired(){

    int64_t now=nowMs();
    std::unique_lock<std::shared_mutex> lock(_shared_mutex);

    for(auto it=_expires.begin();it!=_expires.end();){

        if(it->second <= now){
            _kvstore.erase(it->first);
            it=_expires.erase(it);
        }else{
            it++;
        }

    }


}


void KVStore::set(std::string key,std::string value,bool writeAof){

    std::unique_lock<std::shared_mutex> lock(_shared_mutex);
    _kvstore[key]=value;

    if(_aof&&writeAof){
        std::string str=std::string("SET ")+key+" "+value;
        _aof->append(str);
    }

    _expires.erase(key);

}
std::optional<std::string> KVStore::get(std::string key){

    {
        std::shared_lock<std::shared_mutex> lock(_shared_mutex);
        auto it=_kvstore.find(key);
        if(it ==_kvstore.end()){
            return std::nullopt;
        }
        auto expireit=_expires.find(key);
        if(expireit == _expires.end()){
            return it->second; 
        }
        if(expireit->second > nowMs()){
              return it->second; 
        }
    }

    std::unique_lock<std::shared_mutex> lock(_shared_mutex);
    auto expireit=_expires.find(key);
    if(expireit !=_expires.end() && expireit->second <= nowMs()){
        _kvstore.erase(key);
        _expires.erase(key);
        return std::nullopt;
    }

    auto it=_kvstore.find(key);

    if(it == _kvstore.end()){
        return std::nullopt;
    }
    return it->second;
}

bool KVStore::del(std::string key,bool writeAof){
    std::unique_lock<std::shared_mutex> lock(_shared_mutex);
    auto it=_kvstore.find(key);
    if(it ==_kvstore.end()){
        return false;
    }

    if(_aof&&writeAof){
        _aof->append("DEL "+key);
    }

    int result=_kvstore.erase(key);
    _expires.erase(key);


    return result;
}

std::unordered_map<std::string,std::string> KVStore::snapshot(){
    std::shared_lock<std::shared_mutex> lock(_shared_mutex);
    return _kvstore;
}

bool KVStore::rewriteAOF(){
    if(!_aof){
        return false;
    }

    std::vector<std::string> command;

    {
        std::shared_lock<std::shared_mutex> lock(_shared_mutex);
        if(!_aof->beginRewrite()){
            return false;
        }
        int64_t now=nowMs();

        for(const auto& [key,value]:_kvstore){
            auto expireit=_expires.find(key);
            if(expireit!=_expires.end() && expireit->second<=now){
                continue;
            }

            command.push_back("SET "+key+" "+value);

            if(expireit!=_expires.end()){
                command.push_back("EXPIREAT "+key+" "+std::to_string(expireit->second));
            }

        }
    }

    return _aof->rewrite(command);
}

int64_t KVStore::nowMs(){
    auto time=std::chrono::system_clock::now().time_since_epoch();

    return std::chrono::duration_cast<std::chrono::milliseconds>(time).count();

}

bool KVStore::expire(const std::string&key,int64_t seconds,bool writeAof){
    int64_t expireAtMs=nowMs()+seconds*1000;
    return expireAt(key,expireAtMs,writeAof);
}
bool KVStore::expireAt(const std::string& key,int64_t timestampMs,bool writeAof){

    std::unique_lock<std::shared_mutex> lock(_shared_mutex);

    auto it=_kvstore.find(key);
    if(it == _kvstore.end()){
        return false;
    }

    if(writeAof && _aof){
        _aof->append("EXPIREAT "+key+" "+std::to_string(timestampMs));
    }

    if(timestampMs <= nowMs()){
        _kvstore.erase(key);
        _expires.erase(key);
        return true;
    }
    _expires[key]=timestampMs;
    return true;

}

int64_t KVStore::ttl(const std::string& key){
    std::unique_lock<std::shared_mutex> lock(_shared_mutex);

    auto it=_kvstore.find(key);
    if(it==_kvstore.end()){
        return -2;
    }

    auto expireit=_expires.find(key);
    if(expireit == _expires.end()){
        return -1;
    }

    int64_t remaing=expireit->second-nowMs();
    if(remaing <=0 ){
        _kvstore.erase(key);
        _expires.erase(key);
        return -2;
    }
    return remaing/1000;


}


} // namespace minikv
