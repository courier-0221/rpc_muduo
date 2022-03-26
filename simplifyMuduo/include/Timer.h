#ifndef TIMER_H
#define TIMER_H

#include "Common.h"
#include "ICommonCallback.h"
#include "Timestamp.h"

static uint64_t sequeue;
static uint64_t incrementAndGet() 
{ 
    return __sync_fetch_and_add(&sequeue, 1) + 1; 
}

class Timer
{
public:
    Timer(TimerCallback cb, Timestamp stamp, double interval)
        : _callback(std::move(cb))
        , _stamp(stamp)
        , _interval(interval)
        , _sequence(incrementAndGet())
    {
        //cout << "_sequence " << _sequence << endl;    //debug
    }
    ~Timer() 
    { 
        //cout << "~Timer" << endl; 
    }
    Timestamp getStamp()
    {
        return _stamp;
    }
    uint64_t getsequence()
    {
        return _sequence;
    }
    void timeout()
    {
        if (_callback) _callback();
    }

    bool isRepeat()
    {
        return _interval > 0.0;
    }

    void restart()
    {
        _stamp = Timestamp::nowAfter(_interval);
    }
    
private:
    
    const TimerCallback _callback;
    Timestamp _stamp;
    const double _interval;   //超时时间间隔，=0 为1次性的，=n 每n秒在第一次之后发生
    uint64_t _sequence;   //帧序号 
};


//用于取消定时器
class TimerId : public copyable
{
public:
    TimerId(Timer* timer, int64_t seq)
        : _timer(timer)
        , _sequence(seq)
    {
    }

friend class TimerQueue;    //记录TimerQueue是自己的朋友可以访问私有变量

private:
    Timer* _timer;
    uint64_t _sequence;
};

#endif
