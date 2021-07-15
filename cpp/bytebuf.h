#ifndef __CCNET_BYTEBUF_H__
#define __CCNET_BYTEBUF_H__

#ifdef _MSVC_VER
#pragma execution_character_set("utf-8")
#endif
#include <stdlib.h>
#include <vector>
using namespace std;

class ByteBuf
{
public:

    explicit ByteBuf();
    explicit ByteBuf(int capacity);

    ~ByteBuf(void);
public:
    void writeShort(short value);
    void writeInt(int value);
    void writeInt64(long long value);
    void writeByte(char ch);
    void wirteBytes(char *str);
    void wirteBytes(char*str, int srcIndex, int length);
    void wirteBytes(ByteBuf *buffer);
    void wirteBytes(ByteBuf *buffer, int length);
    void wirteBytes(ByteBuf *buffer, int srcIndex, int length);
    void writeString(const char* str);
    void writeString(const char*str,int length);
    int writeIndex();
    char* flush();
    void release();
public:
    ///read a int  4
    int readInt();
    ///read a short	  2
    short readShort();
    ///read a char   1
    char readByte();
    ///read a int64 8
    long long readInt64();
    //don't deleted this point ,only last call release methed
    const char* readString();
     //don't deleted this point ,only last call release methed
    const char* readString(int length);

    const char* readString(int srcIndex,int length);

    //return writeindex-readindex
    int readableBytes();
     //
    int readIndex();

    void markReadIndex();
    void restReadIndex();

private:
    bool checkRead(int length);
private:
    ///will be handed  data
    char* data;
    //readIndex
    int _readIndex;
    // writeIndex
    int _writeIndex;
    int _readIndexPos;
    vector<char> cache_data;
    vector<char*> release_and_delete;
};

#endif
