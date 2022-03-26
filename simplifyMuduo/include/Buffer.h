#ifndef BUFFER_H
#define BUFFER_H

#include "EndianCode.h"

#include <sys/socket.h>
#include <sys/uio.h>  // readv
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>
#include <string.h>
using namespace std;

/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

class Buffer
{
public:
    static const size_t kCheapPrepend = 8;  //允许在头部追加的直接大小
    static const size_t kInitialSize = 1024;//_buf容器大小 + kCheapPrepend

    Buffer(size_t initialSize = kInitialSize);
    ~Buffer() {}
    
    /* 显示 */
    const char* peek();

    /* 读取，取走后不能再访问 */
    void retrieveAll();
    void retrieve(size_t len);
    string retrieveAllAsString();
    string retrieveAsString(size_t len);
    
    ssize_t readFd(int fd, int* savedErrno);
    
    void ensureWritableBytes(size_t len);
    char* beginWrite(){ return &*_buf.begin() + _writerIndex; }
    void hasWritten(size_t len){ _writerIndex += len; }
    
    /* 插入相关 */
    //插入到writable area
    void append(const string& str);
    void append(const char *data, size_t len);
    //插入到prependable area
    void prepend(const void* data, size_t len);
 
    size_t readableBytes() const { return _writerIndex - _readerIndex; }
    size_t writableBytes() const { return _buf.size() - _writerIndex; }
    size_t prependableBytes() const { return _readerIndex; }
private:
    void makeSpace(size_t len);     //整理空间，如果待写入数据大于buf中可以挤出来的空间（可写+可读[readerIndex]），则扩容；//不大于的话说明可读空间可以做回收，调整可读部分内容到8字节开始的位置
    vector<char> _buf;
    size_t _readerIndex;
    size_t _writerIndex;
};

#endif
