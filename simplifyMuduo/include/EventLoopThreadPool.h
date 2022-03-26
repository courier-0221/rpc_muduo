#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H

#include "Common.h"
#include "Thread.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "Mutex.h"
#include "Condition.h"

class EventLoopThreadPool : noncopyable
{
public:
    EventLoopThreadPool(EventLoop* baseLoop);
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads) { _numThreads = numThreads; }
    void start();
    EventLoop* getNextLoop();

private:
    EventLoop* _baseLoop;	// 与Acceptor所属EventLoop相同
    int _numThreads;  // 线程数
    int _next;  // 新连接到来，所选择的EventLoop对象下标
    vector<EventLoopThreadPtr> _threads;  // IO线程列表
    vector<EventLoop*> _loops; // EventLoop列表 轮叫的时候使用到，各个IO线程中的栈变量，退出时不需要清理
};

#endif
