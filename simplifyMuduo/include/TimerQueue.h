#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include <iostream>
#include <string>
#include <sys/timerfd.h>
#include <inttypes.h>
#include <stdio.h>
#include <strings.h>
#include <vector>
#include <set>
#include <unistd.h>

#include "Common.h"
#include "ICommonCallback.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Timestamp.h"
#include "Timer.h"

//timerfd_create()函数把时间变成了一个描述符，该文件描述符在定时器超时的那一刻变得可读
class TimerQueue : public IChannelCallback
{
public:

    TimerQueue(EventLoop* pLoop);
    ~TimerQueue();
    
    //对外接口
    //interval超时时间间隔，=0 为1次性的，=n 每n秒在第一次之后发生，该接口可以跨线程调用
    TimerId addTimer(TimerCallback cb, Timestamp when, double interval); 
    void cancelTimer(TimerId timerId);

    //timefd对应channel事件处理
    virtual void handleRead();
    virtual void handleWrite() {}
    virtual void handleClose() {}

private:
    //数据结构
    typedef std::pair<Timestamp, Timer*> Entry; //按时间排序
    typedef std::set<Entry> TimerList;  //采用set的原因是即使Timestamp相等Timer*也是不相等的
    
    //添加删除相关
    bool insert(Timer* timer);  //插入到_pTimers容器中
    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);
    
    //超时相关
    vector<TimerQueue::Entry> getExpired(Timestamp now); //返回超时的定时器列表，now这个时刻之前队列中可能有多个定时器超时
    void reset(const vector<Entry>& expired, Timestamp now);    //对超时的定时器重置，如果是重复的定时器
    struct timespec howMuchTimeFromNow(Timestamp when); //计算超时时刻与当前时间的时间差

    //timefd相关
    int createTimerfd();
    void readTimerfd(int timerfd, Timestamp now);
    void resetTimerfd(int timerfd, Timestamp stamp);    //重置定时器超时时间

    int _timerfd;
    TimerList _pTimers; //按照到期时间排序容器
    EventLoop* _pLoop;  
    unique_ptr<Channel> _pTimerfdChannel;
};

#endif
