#include "Buffer.h"

Buffer::Buffer(size_t initialSize) 
    : _buf(kCheapPrepend + initialSize)
    , _readerIndex(kCheapPrepend)
    , _writerIndex(kCheapPrepend)
{}

const char* Buffer::peek()
{
    return &*_buf.begin() + _readerIndex;
}

void Buffer::prepend(const void* data, size_t len)
{
    if (len > prependableBytes())
    {
        cout << "prepend error" << endl;
        return;
    }
    _readerIndex -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d+len, _buf.begin()+_readerIndex);
}

void Buffer::retrieveAll()
{
    _readerIndex = kCheapPrepend;
    _writerIndex = kCheapPrepend;
}

void Buffer::retrieve(size_t len)
{
    if (len < readableBytes())
    {
        _readerIndex += len;
    }
    else
    {
        _readerIndex = kCheapPrepend;
        _writerIndex = kCheapPrepend;
    }
}

string Buffer::retrieveAllAsString()
{
    return retrieveAsString(readableBytes());
}

string Buffer::retrieveAsString(size_t len)
{
    string result(peek(), len);
    retrieve(len);
    return result;
}

void Buffer::append(const char *data, size_t len)
{
    if (len > writableBytes())  //待写入的长度大于buf中可以挤出来的容量
    {
        makeSpace(len); //整理空间，分两种情况1.确实不够扩容 2.容量够，被reader部分占用，则释放该部分
    }
    std::copy(data, data + len, _buf.begin() + _writerIndex);
    _writerIndex += len;
}

void Buffer::append(const string& str)
{
    append(str.c_str(), str.size());
}

void Buffer::makeSpace(size_t len)
{
    //如果待写入数据大于buf中可以挤出来的空间（可写+可读[readerIndex]），则扩容；
    if (writableBytes() + prependableBytes() < len + kCheapPrepend)
    {
        // FIXME: move readable data
        _buf.resize(_writerIndex+len);
    }
    //不大于的话说明可读空间可以做回收，调整可读部分内容到8字节开始的位置
    else
    {
        size_t readable = readableBytes();
        std::copy(_buf.begin()+_readerIndex, _buf.begin()+_writerIndex, _buf.begin()+kCheapPrepend);
        _readerIndex = kCheapPrepend;
        _writerIndex= _readerIndex + readable;
    }
}

void Buffer::ensureWritableBytes(size_t len)
{
    if (writableBytes() < len)
    {
        makeSpace(len);
    }
}

ssize_t Buffer::readFd(int fd, int* savedErrno)
{
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = &*_buf.begin() + _writerIndex;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    // when there is enough space in this buffer, don't read into extrabuf.
    // when extrabuf is used, we read 128k-1 bytes at most.
    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *savedErrno = errno;
    }
    else if (static_cast<size_t>(n) <= writable)
    {
        _writerIndex += n;
        //debug
        //struct MsgHead
        //{
        //    int32_t msgId;
        //    int32_t msgLen;
        //};
        //MsgHead head;
        //memcpy(&head, &*_buf.begin()+_readerIndex, 8);
        //string str(peek() + _readerIndex, n - _readerIndex);
        //cout << "Buffer::readFd readsize " << n << " msgid " << head.msgId << " msglen " << head.msgLen << " str " << str << endl;
        //cout << "_writerIndex " << _writerIndex << " _readerIndex " << _readerIndex << endl;
        //retrieveAll();
    }
    else
    {
        _writerIndex = _buf.size();
        append(extrabuf, n - writable);
    }

  return n;
}

