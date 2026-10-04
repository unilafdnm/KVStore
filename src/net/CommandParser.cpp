#include"CommandParser.h"

#include<vector>
#include<sstream>
#include<iostream>

namespace minikv{

std::optional<minikv::Command> CommandParser::parse(const std::string& line){
    std::stringstream ss(line);
    std::string word;
    std::vector<std::string> res;
    Command command;
    while(ss>>word){
        res.push_back(word);
    }

    if(res.empty()){
        return std::nullopt;
    }

    if(res[0]=="SET"){
        if(res.size()!=3){
            return std::nullopt;
        }
        command.type=CommandType::SET;
        command.key=res[1];
        command.value=res[2];
    }else if(res[0] == "GET"){
        if(res.size()!=2){
            return std::nullopt;
        }
        command.type=CommandType::GET;
        command.key=res[1];
    }else if(res[0] == "DEL"){
        if(res.size()!=2){
            return std::nullopt;
        }
        command.type=CommandType::DEL;
        command.key=res[1];
    }else if(res[0]=="EXPIRE"){
        if(res.size()!=3){
            return std::nullopt;
        }
        command.type=CommandType::EXPIRE;
        command.key=res[1];
        command.value=res[2];
    }else if(res[0] == "TTL"){
        if(res.size()!=2){
            return std::nullopt;
        }
        command.type=CommandType::TTL;
        command.key=res[1];
    }else if(res[0] == "EXPIREAT"){
        if(res.size()!=3){
            return std::nullopt;
        }
        command.type=CommandType::EXPIREAT;
        command.key=res[1];
        command.value=res[2];
    }else{
        return std::nullopt;
    }



    for(int i=0;i<res.size();i++){
        if(i==0){
            
        }else if(i==1){
            command.key=res[i];
        }else{
            command.value=res[i];
        }

    }

    return command;

}

}