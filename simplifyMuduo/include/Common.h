#ifndef COMMON_H
#define COMMON_H

#include <memory>
#include <iostream>
using namespace std;

#define MAX_LINE        100     //服务端读取客户端发送到数据的临时缓冲区大小，单位：字节
#define MAX_EVENTS      500     //满足监听条件的最大事件个数
#define MAX_LISTENFD    5       //同一时间连接的客户端个数

//channel 对应的事件类型
const int kNew = -1;            //增加事件
const int kAdded = 1;           //修改事件
const int kDeleted = 2;         //删除事件

class IChannelCallback;
class IAcceptorCallback;
class Channel;
class Acceptor;
class TcpConnection;
class EventLoop;
class Epoll;
class IMuduoUser;
class Buffer;
class TimerQueue;
class Timestamp;
class Timer;
class Thread;
class ThreadPool;

class noncopyable
{
 public:
  noncopyable(const noncopyable&) = delete;
  void operator=(const noncopyable&) = delete;

 protected:
  noncopyable() = default;
  ~noncopyable() = default;
};

class copyable
{
 protected:
  copyable() = default;
  ~copyable() = default;
};

typedef std::function<void()> TimerCallback;
typedef shared_ptr<TcpConnection> TcpConnectionPtr;

#endif
