#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H

#include "Common.h"
#include "Thread.h"
#include "EventLoop.h"
#include "Mutex.h"
#include "Condition.h"

class EventLoopThread;
typedef unique_ptr<EventLoopThread> EventLoopThreadPtr;

class EventLoopThread : noncopyable
{
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* startLoop();	// 启动线程，该线程就成为了IO线程

private:
    void threadFunc();  // 线程函数
    EventLoop* _loop; 
    Thread _thread;
    MutexLock _mutex;
    Condition _cond;
};


#endif

