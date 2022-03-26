#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop)
  : _baseLoop(baseLoop)
  , _numThreads(0)
  , _next(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    if (!_threads.empty())
    {
        vector<EventLoopThreadPtr>::iterator it;
        for (it = _threads.begin(); it != _threads.end();)
        {
            _threads.erase(it);
        }
    }
}

void EventLoopThreadPool::start()
{
    for (int i = 0; i < _numThreads; ++i)
    {
        EventLoopThreadPtr t(new EventLoopThread());
        _loops.push_back(t->startLoop());	// 启动EventLoopThread线程
        _threads.push_back(std::move(t));
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    EventLoop* loop = _baseLoop;    //没有启动多线程时使用主的eventloop

    // 如果_loops为空，则loop指向_baseLoop
    // 如果不为空，按照round-robin（RR，轮叫）的调度方式选择一个EventLoop
    if (!_loops.empty())
    {
        // round-robin
        loop = _loops[_next];
        ++_next;
        //轮转
        if (static_cast<size_t>(_next) >= _loops.size())
        {
            _next = 0;
        }
    }
    return loop;
}

