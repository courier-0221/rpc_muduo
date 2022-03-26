#include "EventLoop.h"

EventLoop::EventLoop()
    :_quit(false)
    ,_callingPendingFunctors(false)
    ,_pPoller(new Epoll())
    ,_threadId(CurrentThread::tid())
    ,_pTimerQueue(new TimerQueue(this))
    ,_eventfd(createEventfd())
    ,_pEventfdChannel(new Channel(this, _eventfd))
{
    _pEventfdChannel->setCallback(this);
    _pEventfdChannel->enableReading();
    //for debug
    cout << "EventLoop::EventLoop() create tid = " << CurrentThread::tid() << endl;
}

void EventLoop::loop()
{
    cout << "EventLoop::loop() start tid = " << CurrentThread::tid() << endl;
    while(!_quit)
    {
        vector<Channel*> activeChannels;
        _pPoller->poll(&activeChannels);
        for (auto *currentActiveChannel : activeChannels)
        {
            currentActiveChannel->handleEvent();
        }

        doPendingFunctors();
    }
}

void EventLoop::update(Channel* pChannel)
{
    _pPoller->update(pChannel);
}

void EventLoop::quit()
{
    _quit = true;
    if (!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::queueInLoop(funcCB cb)
{
    {
        MutexLockGuard guard(_mutex);
        _pendingFunctors.push_back(cb);
    }

    if (!isInLoopThread() || _callingPendingFunctors)
    {
        wakeup();
    }
}

//在io线程中做一些计算线程中才会做的事情
void EventLoop::runInLoop(funcCB cb)
{
    if (isInLoopThread())   //如果调用该方法的线程等于EventLoop中的线程则立即执行
    {
        if (cb) cb();
    }
    else                    //否则加入到队列中
    {
        queueInLoop(cb);
    }
}

bool EventLoop::isInLoopThread()
{
    return _threadId == CurrentThread::tid();
}

void EventLoop::wakeup()
{
    uint64_t wakeup = 1;
    ssize_t n = ::write(_eventfd, &wakeup, sizeof wakeup);
    if (n != sizeof wakeup)
    {
        cout << "EventLoop::wakeup() writes " << n << " bytes instead of 8" << endl;
    }
}

int EventLoop::createEventfd()
{
   int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
   if (evtfd < 0)
   {
       cout << "Failed in eventfd" << endl;
   }
   return evtfd;
}

void EventLoop::doPendingFunctors()
{
    vector<funcCB> tempRuns;
    _callingPendingFunctors = true;
    {
        MutexLockGuard guard(_mutex);
        tempRuns.swap(_pendingFunctors);
    }

    for (const funcCB& cb : tempRuns)
    {
        cb();
    }
    _callingPendingFunctors = false;
}

void EventLoop::handleRead()
{
    uint64_t wakeup = 1;
    ssize_t n = ::read(_eventfd, &wakeup, sizeof wakeup);
    if (n != sizeof wakeup)
    {
        cout << "EventEventLoop::handleRead() reads " << n << " bytes instead of 8" << endl;
    }
}

TimerId EventLoop::runAt(Timestamp when, TimerCallback cb)
{
    return _pTimerQueue->addTimer(std::move(cb), when, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback cb)
{
    return _pTimerQueue->addTimer(std::move(cb), Timestamp::nowAfter(delay), 0.0);
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb)
{
    return _pTimerQueue->addTimer(std::move(cb), Timestamp::nowAfter(interval), interval);
}

void EventLoop::cancelTimer(TimerId timerId)
{
    _pTimerQueue->cancelTimer(timerId);
}