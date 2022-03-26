#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <sys/eventfd.h>
#include "Common.h"
#include "ICommonCallback.h"
#include "Mutex.h"
#include "Channel.h"
#include "Epoll.h"
#include "Timer.h"
#include "TimerQueue.h"
#include "Timestamp.h"
#include "CurrentThread.h"
#include <vector>

//EventLoop 和 Epoll是一一对应的 创建并运行了该类的线程称之为IO线程
//one loop per thread(IO线程) + pthreadpool(计算线程池)
//每个eventloop监听两个文件描述符，eventfd+timefd
class EventLoop : noncopyable, public IChannelCallback
{
public:
    typedef std::function<void()> funcCB;
    EventLoop();
    ~EventLoop() { cout << "EventLoop::~EventLoop()" << endl; }

    /* Epoll相关 */
    void loop();                    //epoll_wait监听到达的事件并做相应处理
    void update(Channel* pChannel); //设置channel相应的监听事件到epoll中，实际在channel中调用该方法
    void quit();                    //退出
    
    /* 定时器相关 */
    TimerId runAt(Timestamp when, TimerCallback cb);
    TimerId runAfter(double delay, TimerCallback cb);
    TimerId runEvery(double interval, TimerCallback cb);
    void cancelTimer(TimerId timerId);
    
    /* eventfd相关 */
    void queueInLoop(funcCB cb);   //向异步任务队列加入任务
    void runInLoop(funcCB cb);     //尝试在Loop中立即执行cb（实现计算线程的功能做一些计算），如果不是loop线程则加入到异步任务队列中
    bool isInLoopThread();         //当前调用线程是不是和EventLoop线程相等
    virtual void handleRead();
    virtual void handleWrite() {}
    virtual void handleClose() {}
private:
    /* eventfd相关 */
    void wakeup();
    int createEventfd();
    void doPendingFunctors();       //执行异步任务队列任务
    
    bool _quit;
    bool _callingPendingFunctors;  //是否正在执行异步队列中的任务
    unique_ptr<Epoll> _pPoller;
    const pid_t _threadId;
    unique_ptr<TimerQueue> _pTimerQueue;
    int _eventfd;                           //eventfd用于异步处理一些相应，比如三个半事件中的结束连接和写完成
    unique_ptr<Channel> _pEventfdChannel;   //eventfd对应的channel
    
    MutexLock _mutex;               //配合异步任务队列
    vector<funcCB> _pendingFunctors;//异步任务队列
};

#endif
