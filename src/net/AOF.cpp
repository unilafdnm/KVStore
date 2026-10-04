#include"AOF.h"
#include"KVStore.h"
#include"CommandParser.h"
#include"Command.h"
#include<optional>
#include<thread>
#include<chrono>
namespace minikv{

AOF::AOF(const std::string& filename)
    :_filename(filename)
{
    _file.open(filename,std::ios_base::app);
    if(!_file.is_open()){
        std::cerr<<"open aof failed\n";
    }
}

void AOF::append(const std::string& command){

    std::lock_guard<std::mutex> lock(_mutex);
    _file<<command<<'\n';
    _file.flush();
    if(_rewriting){
        _rewritrBuffer.push_back(command);
    }
}

void AOF::load(KVStore& kvstore){
    std::ifstream infile(_filename);
    if(!infile.is_open()){
        std::cerr<<"replay aof open failed\n";
        return;
    }

    std::string line;
    CommandParser parser;
    while(std::getline(infile,line)){
        std::optional<Command> command=parser.parse(line);

        std::cout<<"key="<<command->key<<'\n';

        if(!command.has_value()){
            continue;
        }
        if(command->type==CommandType::SET){
            kvstore.set(command->key,command->value,false);
        }else if(command->type == CommandType::DEL){
            kvstore.del(command->key,false);
        }else{
            continue;
        }

    }


}

bool AOF::rewrite(const std::unordered_map<std::string,std::string>& snapshot){

    std::string tempfileName=_filename+".tmp";
    std::ofstream tempFile(tempfileName,std::ios_base::trunc);
    if(!tempFile.is_open()){
        std::cerr<<tempfileName<<" open failed\n";
        std::lock_guard<std::mutex> lock(_mutex);
        _rewriting=false;
        _rewritrBuffer.clear();
        return false;
    }

    for(const auto& [key,value] : snapshot){
        tempFile<<"SET "<<key<<" "<<value<<'\n';
       
    }

    {
        std::lock_guard<std::mutex> lock(_mutex);
        for(const auto& str:_rewritrBuffer){
            tempFile<<str<<'\n';
        }

    }


    tempFile.flush();
    tempFile.close();

    std::lock_guard<std::mutex> lock(_mutex);
    _file.close();

    if(rename(tempfileName.c_str(),_filename.c_str())!=0){
        std::cerr<<"rename failed\n";
        _file.open(_filename,std::ios_base::app);
        _rewriting=false;
        _rewritrBuffer.clear();
        return false;
    }
    _file.open(_filename,std::ios_base::app);

    _rewriting=false;
    _rewritrBuffer.clear();
    return true;

}

bool AOF::beginRewrite(){
    std::lock_guard<std::mutex> lock(_mutex);
    if(_rewriting){
        return false;
    }

    _rewriting=true;
    _rewritrBuffer.clear();
    return true;
}




}