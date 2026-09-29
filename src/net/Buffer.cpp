#include"Buffer.h"

#include<algorithm>
#include<cstring>
#include<iostream>

namespace minikv{

Buffer::Buffer(size_t initialSize)
    :_buffer(initialSize),_readIndex(0),_writeIndex(0)
{
    
}
Buffer::~Buffer(){

}

std::size_t Buffer::readableBytes()const{
    return _writeIndex-_readIndex;
}
std::size_t Buffer::writeableBytes()const{
    return _buffer.size()-_writeIndex;
}

const char* Buffer::peek()const{
    return _buffer.data()+_readIndex;
}

void Buffer::retrieve(std::size_t len){

    if(len<readableBytes()){
        _readIndex+=len;
    }else{
        retrieveAll();
    }

}
void Buffer::retrieveAll(){

    _readIndex=0;
    _writeIndex=0;
}

std::string Buffer::retrieveAsString(std::size_t len){
    len=std::min(len,readableBytes());

    std::string result(peek(),len);

    retrieve(len);
    return result;

}
std::string Buffer::retrieveAllAsString(){
    return retrieveAsString(readableBytes());
}

void Buffer::append(const char* data,std::size_t len){
    ensureWriteableBytes(len);

    std::memcpy(_buffer.data()+_writeIndex,data,len);
    _writeIndex+=len;


}


void Buffer::ensureWriteableBytes(std::size_t len){

    if(len <= writeableBytes()){
        return;
    }

    std::size_t readable=readableBytes();

    if(_readIndex+writeableBytes() >=len){
         std::memmove(_buffer.data(),_buffer.data()+_readIndex,readable);
        _readIndex=0;
        _writeIndex=readable;
    }else{
        _buffer.resize(_writeIndex+len);
    }

   

}

const char* Buffer::findCRLF()const{

    const char* begin=peek();
    const char* end=_buffer.data()+_writeIndex;


    for(const char* p=begin;p+1<end;++p){
        if(p[0]=='\r' && p[1]=='\n'){
            return p;
        }

    }

    return nullptr;

}


}