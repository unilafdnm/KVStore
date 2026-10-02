#include"CommandParser.h"

#include<vector>
#include<sstream>

namespace minikv{

std::optional<minikv::Command> CommandParser::parse(const std::string& line){
    std::stringstream ss(line);
    std::string word;
    std::vector<std::string> res;
    Command command;
    while(ss>>word){
        res.push_back(word);
    }

    if(res.size()>3 || res.size()<=1){
        return std::nullopt;
    }

    for(int i=0;i<res.size();i++){
        if(i==0){
            if(res[i]=="SET"){
                if(res.size()<3){
                    return std::nullopt;
                }
                command.type=CommandType::SET;
            }else if(res[i] == "GET"){
                if(res.size()!=2){
                    return std::nullopt;
                }
                command.type=CommandType::GET;
            }else if(res[i] == "DEL"){
                if(res.size()!=2){
                    return std::nullopt;
                }
                command.type=CommandType::DEL;
            }else{
                return std::nullopt;
            }
        }else if(i==1){
            command.key=res[i];
        }else{
            command.value=res[i];
        }

    }

    return command;

}

}