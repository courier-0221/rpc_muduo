#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "Thread.h"
#include "Common.h"
#include "BlockingQueue.h"
#include <vector>
#include <memory>
using namespace std;

//理论线程池实现思路：使用一个任务队列+一个互斥锁+两个条件变量来实现(不为满和不为空)
//简易版实现思路：只使用阻塞队列中的条件变量(不为空)来实现，弊端为没限制任务队列的大小&不可以回收线程
//写的不好待后续优化
class ThreadPool : noncopyable
{
public:
    typedef std::function<void ()> funcCB;
    ThreadPool(string name = "ThreadPool") : _pollName(name) {};
    ~ThreadPool() {};
    void start(int numThreads)
    {
        _threads.reserve(numThreads);
        for (int i = 0 ; i < numThreads; i++)
        {
            char id[32];
            snprintf(id, sizeof id, "%d", i+1);
            _threads.emplace_back(new Thread(std::bind(&ThreadPool::runInThread, this), _pollName+id));
            _threads[i]->start();
        }
    }
    void addTask(funcCB cb)
    {
        if (_threads.empty())   //线程队列为空则直接执行
        {
            cb();
        }
        else
        {
            _tasks.put(cb);
        }
    }
    
private:
    void runInThread()   //线程处理函数 被唤醒从_tasks队列中拿任务处理
    {
        while (true)
        {
            funcCB cb = _tasks.take();
            if (cb) cb();
        }
    }
    string _pollName;
    BlockingQueue<funcCB> _tasks; //使用阻塞队列实现，任务加入队列中的同时唤醒线程池中的线程执行
    vector<std::unique_ptr<Thread>> _threads;
};

#endif