#include"KVStore.h"

namespace minikv
{
void KVStore::set(std::string key,std::string value){

    std::unique_lock<std::shared_mutex> lock(_shared_mutex);
    _kvstore[key]=value;
}
std::optional<std::string> KVStore::get(std::string key){

    std::shared_lock<std::shared_mutex> lock(_shared_mutex);
    auto it=_kvstore.find(key);
    if(it ==_kvstore.end()){
        return std::nullopt;
    }
    return it->second; 

}
bool KVStore::del(std::string key){
    // std::unique_lock<std::mutex> lock(_mutex);
    std::unique_lock<std::shared_mutex> lock(_shared_mutex);
    auto it=_kvstore.find(key);
    if(it ==_kvstore.end()){
        return false;
    }
    int result=_kvstore.erase(key);
    return result;
}


} // namespace minikv
