#pragma once

#include<cstddef>
#include<string>
#include<vector>

namespace minikv{

class Buffer{

public:
    Buffer(size_t initialSize=1024);
    ~Buffer();

    std::size_t readableBytes()const;
    std::size_t writeableBytes()const;

    const char* peek()const;

    void retrieve(std::size_t len);
    void retrieveAll();

    std::string retrieveAsString(std::size_t len);
    std::string retrieveAllAsString();

    void append(const char* data,std::size_t len);

    const char* findCRLF()const;


private:
    void ensureWriteableBytes(std::size_t len);


private:
    std::vector<char> _buffer;
    std::size_t _readIndex;
    std::size_t _writeIndex;


};

}