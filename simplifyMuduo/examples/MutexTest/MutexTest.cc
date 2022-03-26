#include "Thread.h"
#include "CurrentThread.h"
#include "Mutex.h"
#include "Condition.h"
#include "Timestamp.h"

#include <memory>
#include <string>
#include <unistd.h>
#include <iostream>
#include <vector>
using namespace std;

MutexLock g_mutex;
vector<int> g_vec;
const int kCount = 1*1000*1000;

void UserTask()
{
    cout<< "son tid = " << CurrentThread::tid() << endl;
    for (int i = 0; i < kCount; ++i)
    {
        MutexLockGuard lock(g_mutex);
        g_vec.push_back(i);
    }
}

int main()
{
    cout << "main pid =" << ::getpid() << " tid = " << CurrentThread::tid() << endl;
    cout << "sizeof pthread_mutex_t: " << sizeof(pthread_mutex_t) << endl;
    cout << "sizeof Mutex: " << sizeof(MutexLock) << endl;
    cout << "sizeof pthread_cond_t: " << sizeof(pthread_cond_t) << endl;
    cout << "sizeof Condition: " << sizeof(Condition) << endl;

    //给定vector容量
    const int kMaxThreads = 8;
    g_vec.reserve(kMaxThreads * kCount);

    //不带锁 单线程向vec中加入元素
    Timestamp start(Timestamp::now());
    for (int i = 0; i < kCount; ++i)
    {
        g_vec.push_back(i);
    }
    cout << "single thread without lock " << timeDifference(Timestamp::now(), start) << endl;

    //带锁 单线程向vec中加入元素
    start = Timestamp::now();
    UserTask();
    cout << "single thread with lock " << timeDifference(Timestamp::now(), start) << endl;
    
    //带锁 多线程向vec中加入元素
    for (int nthreads = 1; nthreads < kMaxThreads; nthreads++)
    {
        vector<std::unique_ptr<Thread>> threads;    //外层for循环执行完一次才会执行析构
        //vector<Thread*> threads;
        g_vec.clear();
        start = Timestamp::now();
        for (int i = 0; i < nthreads; ++i)
        {
            threads.emplace_back(new Thread(UserTask));
            threads.back()->start();
        }
        sleep(3);
        for (int i = 0; i < nthreads; ++i)
        {
            threads[i]->join();
        }
        cout << nthreads << " thread(s) with lock " << timeDifference(Timestamp::now(), start) << endl;
   }

    return 0;
}
