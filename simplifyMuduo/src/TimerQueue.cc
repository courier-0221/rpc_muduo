#include "TimerQueue.h"

TimerQueue::TimerQueue(EventLoop *pLoop)
    :_timerfd(createTimerfd())
    ,_pLoop(pLoop)
    ,_pTimerfdChannel(new Channel(_pLoop, _timerfd))
{
    _pTimerfdChannel->setCallback(this);
    _pTimerfdChannel->enableReading();
}

TimerQueue::~TimerQueue()
{
    _pTimerfdChannel->setIndex(kDeleted);
    _pTimerfdChannel->disableAll();
    ::close(_timerfd);
    for (const Entry& timer : _pTimers)
    {
        delete timer.second;
    }
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    //插入一个定时器，有可能会使得最早到期的定时器发生改变
    bool earliestChanged = insert(timer);  //如果插入的时间比队列中维护的还要早，返回true
    if (earliestChanged)
    {
        //重置定时器的超时时刻timerfd_settime
        resetTimerfd(_timerfd, timer->getStamp());
    }
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
    //构造要删除的元素
    Entry timer(timerId._timer->getStamp(), timerId._timer);
    TimerList::iterator it = _pTimers.find(timer);  //查找
    if (it != _pTimers.end())
    {
        _pTimers.erase(it);//删除
        delete it->second;
    }
}

TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{
    Timer* addTimer = new Timer(std::move(cb), when, interval);
    _pLoop->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, addTimer));
    return TimerId(addTimer, addTimer->getsequence());
}

void TimerQueue::cancelTimer(TimerId timerId)
{
    _pLoop->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::handleRead()
{
    Timestamp now(Timestamp::now());
    readTimerfd(_timerfd, now);

    //获取该时刻之前的所有定时器列表(超时定时器列表，从_pTimers中获取)
    vector<Entry> expired = getExpired(now);
    vector<Entry>::iterator it;
    for (it = expired.begin(); it != expired.end(); ++it)
    {
        it->second->timeout();
    }
    reset(expired, now);    //如果不是一次性的定时器需要进行重启
}

int TimerQueue::createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
    {
        cout << "failed in timerfd_create" << endl;
    }
    return timerfd;
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    //构造比对元素
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    //返回第一个未到期的Timer的迭代器，此处会找到_pTimers容器中第一个大于now的时间戳，，，end->first > now
    //lower_bound返回第一个>=sentry的iterator，比对两个值，first时间戳和second地址
    //比对机制为如果有一个元素的first>=now（相等），会继续比对second，没有元素可以大的过UINTPTR_MAX，所以不成立继续查找
    //如果有一个元素的first>now，则不会比对second，此时返回
    TimerList::iterator end = _pTimers.lower_bound(sentry);
    copy(_pTimers.begin(), end, back_inserter(expired));    //将到期的定时器插入到expired中，end之前的
    _pTimers.erase(_pTimers.begin(), end);  //清除
    return expired; //这里不会拷贝构造，因为有rvo的优化，不会影响性能
}

void TimerQueue::readTimerfd(int timerfd, Timestamp now)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
    //cout << "TimerQueue::handleRead() " << howmany << " at " << now.toString() << endl;
    if (n != sizeof(howmany))
    {
        cout << "Timer::readTimerfd() error " << endl;
    }
}

void TimerQueue::reset(const vector<Entry>& expired, Timestamp now)
{
    vector<Entry>::const_iterator it;
    for (it = expired.begin(); it != expired.end(); ++it)
    {
        //如果是重复的定时器并且是未取消定时器，则重启该定时器
        if (it->second->isRepeat())
        {
            it->second->restart();   //重新计算
            insert(it->second);
        }
        else
        {
            delete it->second;      //一次性的定时器则删除
        }
    }

    Timestamp nextExpire;
    if (!_pTimers.empty())
    {
        // 获取最早到期的定时器超时时间
        nextExpire = _pTimers.begin()->second->getStamp();
    }
    if (nextExpire.valid())
    {
        // 重置定时器的超时时刻(timerfd_settime)
        resetTimerfd(_timerfd, nextExpire);
    }
}

void TimerQueue::resetTimerfd(int timerfd, Timestamp stamp)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof(newValue));
    bzero(&oldValue, sizeof(oldValue));
    newValue.it_value = howMuchTimeFromNow(stamp);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret)
    {
        cout << "TimerQueue::resetTimerfd error" << endl;
    }
}

bool TimerQueue::insert(Timer* timer)
{
    bool earliestChanged = false;   //最早到期时间是否改变
    Timestamp when = timer->getStamp();
    TimerList::iterator it = _pTimers.begin();
    //如果_pTimers为空或者when小于_pTimers中的最早到期时间
    if (it == _pTimers.end() || when < it->first)
    {
        earliestChanged = true;
    }
    //插入到_pTimers中
    pair<TimerList::iterator, bool> result = _pTimers.insert(Entry(when, timer));
    if (!(result.second))
    {
        cout << "TimerQueue::insert _pTimers.insert() error " << endl;
    }

    return earliestChanged;
}

struct timespec TimerQueue::howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.microSecondsSinceEpoch()
        - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
            microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
            (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}
