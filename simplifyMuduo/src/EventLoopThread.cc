#include "EventLoopThread.h"

EventLoopThread::EventLoopThread()
    : _loop(NULL)
    , _thread(std::bind(&EventLoopThread::threadFunc, this))
    , _mutex()
    , _cond(_mutex)
{
}

EventLoopThread::~EventLoopThread()
{
    _loop->quit();		// 退出IO线程，让IO线程的loop循环退出，从而退出了IO线程
    _thread.join();
}

EventLoop* EventLoopThread::startLoop()
{
    _thread.start();
    {
        MutexLockGuard lock(_mutex);
        while (_loop == NULL)
        {
            _cond.wait();
        }
    }
    return _loop;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;
    {
        MutexLockGuard lock(_mutex);
        // loop_指针指向了一个栈上的对象，threadFunc函数退出之后，这个指针就失效了
        // threadFunc函数退出，就意味着线程退出了，EventLoopThread对象也就没有存在的价值了。
        // 因而不会有什么大的问题
        _loop = &loop;
        _cond.notify();
    }
    loop.loop();
}