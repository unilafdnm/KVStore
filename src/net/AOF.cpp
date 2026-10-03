#include"AOF.h"
#include"KVStore.h"
#include"CommandParser.h"
#include"Command.h"
#include<optional>

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



}