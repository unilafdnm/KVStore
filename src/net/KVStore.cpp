#include"KVStore.h"
#include"AOF.h"
namespace minikv
{

KVStore::KVStore(AOF* aof)
    :_aof(aof)
{

}

void KVStore::set(std::string key,std::string value,bool writeAof){

    std::unique_lock<std::shared_mutex> lock(_shared_mutex);
    _kvstore[key]=value;

    if(_aof&&writeAof){
        std::string str=std::string("SET ")+key+" "+value;
        _aof->append(str);
    }


}
std::optional<std::string> KVStore::get(std::string key){

    std::shared_lock<std::shared_mutex> lock(_shared_mutex);
    auto it=_kvstore.find(key);
    if(it ==_kvstore.end()){
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

    std::unordered_map<std::string,std::string> snapshot;

    {
        std::shared_lock<std::shared_mutex> lock(_shared_mutex);
        if(!_aof->beginRewrite()){
            return false;
        }
        snapshot=_kvstore;
    }

    return _aof->rewrite(snapshot);
}



} // namespace minikv
