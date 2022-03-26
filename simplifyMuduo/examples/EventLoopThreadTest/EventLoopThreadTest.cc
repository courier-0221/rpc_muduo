#include "Thread.h"
#include "CurrentThread.h"
#include "Mutex.h"
#include "Condition.h"
#include "Timestamp.h"
#include "EventLoopThread.h"

#include <memory>
#include <string>
#include <unistd.h>
#include <iostream>
#include <vector>
using namespace std;

void runInThread()
{
    cout << "runInThread(): pid = " << getpid() << " tid = " << CurrentThread::tid() << endl;
}

int main()
{
    cout << "main pid =" << ::getpid() << " tid = " << CurrentThread::tid() << endl;
    cout << "sizeof pthread_mutex_t: " << sizeof(pthread_mutex_t) << endl;
    cout << "sizeof Mutex: " << sizeof(MutexLock) << endl;
    cout << "sizeof pthread_cond_t: " << sizeof(pthread_cond_t) << endl;
    cout << "sizeof Condition: " << sizeof(Condition) << endl;

    EventLoopThread loopThread;
    EventLoop* loop = loopThread.startLoop();
    // 异步调用runInThread，即将runInThread添加到loop对象所在IO线程，让该IO线程执行
    loop->runInLoop(runInThread);
    sleep(1);
    // runAfter内部也调用了runInLoop，所以这里也是异步调用
    loop->runAfter(1, runInThread);
    loop->runAfter(2, runInThread);
    loop->runAfter(3, runInThread);
    sleep(3);
    loop->quit();

    cout << "exit main()" << endl;

    return 0;
}
