#ifdef _MSVC_VER
#pragma execution_character_set("utf-8")
#endif
#include "bytebuf.h"
#include <exception>
#include <string.h>


//stream动态缓冲区的初始化大小
#define BYTE_BUFFER_SIZE 512
ByteBuf::~ByteBuf(){}

ByteBuf::ByteBuf():ByteBuf(BYTE_BUFFER_SIZE)
{
}
ByteBuf::ByteBuf(int capacity)
    :data(nullptr),_readIndex(0),_writeIndex(0),_readIndexPos(0)
{
    this->cache_data.reserve(capacity);
}


char*ByteBuf::flush(){
    int _size = this->readableBytes();
    if (_size != 0)
    {
        int nIdx = 0;
        this->data = new char[_size];
        for(auto iter = cache_data.begin(); iter != cache_data.end(); iter++, nIdx++)
        {
            this->data[nIdx] = *iter;
        }
        return this->data;		   //this data will be  dispatcher by logic		, be careful    at last you must be call release
    }
    return NULL;
}


void ByteBuf::release(){
    if (data != NULL)
    {
        delete data;
    }
    data = nullptr;
    this->_readIndex = 0;
    this->_writeIndex = 0;
    cache_data.clear();

    if (!release_and_delete.empty())
    {
        for (auto iter = begin(release_and_delete); iter != end(release_and_delete); ++iter)
        {
            delete *iter;
            *iter = nullptr;
        }
    }
    release_and_delete.clear();
}


void ByteBuf::writeShort(short num)
{
    this->cache_data.push_back( ((0xff00 & num) >> 8) );
    this->cache_data.push_back( (0xff & num) );
    _writeIndex += 0x2;
}


void ByteBuf::writeInt(int num)
{
    this->cache_data.push_back( ((0xff000000 & num) >> 24) );
    this->cache_data.push_back( ((0xff0000 & num) >> 16) );
    this->cache_data.push_back( ((0xff00 & num) >> 8) );
    this->cache_data.push_back( (0xff & num) );
    _writeIndex += 0x4;
}

void ByteBuf::writeInt64(long long num)
{
    this->cache_data.push_back( ((0xff00000000000000 & num) >> 56) );
    this->cache_data.push_back( ((0xff000000000000 & num) >> 48) );
    this->cache_data.push_back( ((0xff0000000000 & num) >> 40) );
    this->cache_data.push_back( ((0xff00000000 & num) >> 32) );
    this->cache_data.push_back( ((0xff000000 & num) >> 24) );
    this->cache_data.push_back( ((0xff0000 & num) >> 16) );
    this->cache_data.push_back( ((0xff00 & num) >> 8) );
    this->cache_data.push_back( (0xff & num) );
    _writeIndex += 0x8;
}


void ByteBuf::writeByte(char ch)
{
    this->cache_data.push_back(ch);
    _writeIndex += 0x1;
}

void ByteBuf::wirteBytes(char *str)
{
    wirteBytes(str, 0, strlen(str));
}

void ByteBuf::wirteBytes(char*str, int srcIndex, int length)
{
    for (int index = srcIndex; index != length;++index)
    {
        this->cache_data.push_back(str[index]);
    }
    _writeIndex += length;
}

void ByteBuf::wirteBytes(ByteBuf *buffer)
{
    wirteBytes(buffer, buffer->_writeIndex);
    buffer->release();
}

void ByteBuf::wirteBytes(ByteBuf *buffer, int length)
{
    wirteBytes(buffer,0,buffer->_writeIndex);
    buffer->release();
}

void ByteBuf::wirteBytes(ByteBuf *buffer, int srcIndex, int length)
{
    for (int index = srcIndex; index != length;++index)
    {
        this->cache_data.push_back(buffer->cache_data[index]);
    }
    _writeIndex += length;
    buffer->release();
}

void ByteBuf::writeString(const char* str)
{
    auto _size = strlen(str);
    for (int index = 0; index != _size; ++index){
        this->cache_data.push_back(str[index]);
    }
    _writeIndex += _size;
}

void ByteBuf::writeString(const char*str, int length)
{
    for (int index = 0; index != length; ++index){
        this->cache_data.push_back(str[index]);
    }
    _writeIndex += length;
}

int ByteBuf::writeIndex(){
    return this->_writeIndex;
}


int ByteBuf::readIndex()
{
    return _readIndex;
}

void ByteBuf::markReadIndex()
{
    _readIndexPos = _readIndex;
}

void ByteBuf::restReadIndex()
{
    for(auto i=_readIndexPos;i<_readIndex;++i)
    {
        auto value = this->release_and_delete[i];
    }
}

int ByteBuf::readInt()
{
    if(!checkRead(4))
        return 0;
    int addr = cache_data[_readIndex + 3] & 0xff;
    addr |= ((cache_data[_readIndex + 2] << 8) & 0xff00);
    addr |= ((cache_data[_readIndex + 1] << 16) & 0xff0000);
    addr |= ((cache_data[_readIndex] << 24) & 0xff000000);
    _readIndex += 4;
    return addr;
}


short ByteBuf::readShort()
{
    if(!checkRead(2)) return 0;
    short addr = cache_data[_readIndex + 1] & 0xff;
    addr |= ((cache_data[_readIndex] << 8) & 0xff00);
    _readIndex += 2;
    return addr;
}

char ByteBuf::readByte()
{
    if(!checkRead(1)) return 0;
    char addr = cache_data[_readIndex];
    _readIndex += 1;
    return addr;
}

long long ByteBuf::readInt64()
{
    if(!checkRead(8)) return 0;
    long long addr = cache_data[_readIndex + 7] & 0xff;
    addr |= ((cache_data[_readIndex + 6] << 8)  & 0xff00);
    addr |= ((cache_data[_readIndex + 5] << 16) & 0xff0000);
    addr |= ((cache_data[_readIndex + 4] << 24) & 0xff000000);
    addr |= ((cache_data[_readIndex + 3] << 32) & 0xff00000000);
    addr |= ((cache_data[_readIndex + 2] << 40) & 0xff0000000000);
    addr |= ((cache_data[_readIndex + 1] << 48) & 0xff000000000000);
    addr |= ((cache_data[_readIndex] << 56)     & 0xff00000000000000);
    _readIndex += 8;
    return addr;
}

const char* ByteBuf::readString()
{
    return readString(_writeIndex);
}
const char* ByteBuf::readString(int length)
{
    if (length <= 0) return nullptr;
    char * temp = new char[length];
    for (int index = 0; index != length; ++index)
    {
        temp[index] = this->cache_data[_readIndex];
        ++_readIndex;
    }
    release_and_delete.push_back(temp);
    return temp;
}

const char* ByteBuf::readString(int srcIndex, int length)
{
    if (length <= 0) return nullptr;

    char * temp = new char[length];
    for (int index = 0; index != length ; ++index)
    {
        temp[index] = this->cache_data[srcIndex];	 //指定位置开始读取数据
        ++srcIndex;
    }
    _readIndex += length;
    release_and_delete.push_back(temp);
    return temp;
}

int ByteBuf::readableBytes()
{
    auto _size = _writeIndex - _readIndex;
    return _size < 0 ? 0 : _size;
}

bool ByteBuf::checkRead(int length)
{
    return _writeIndex>=length;
//  if (_writeIndex<length)
//  {
//      throw std::exception("_writeIndex less can read length!");
//  }
}
