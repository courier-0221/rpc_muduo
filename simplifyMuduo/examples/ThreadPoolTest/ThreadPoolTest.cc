#include "CurrentThread.h"
#include "ThreadPool.h"

#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <stdio.h>
using namespace std;

void UserThreadTask01()
{
    printf("UserThreadTask01 tid=%d\n", CurrentThread::tid());
}

void UserThreadTask02(const string& str)
{
    printf("UserThreadTask02 tid=%d str=%s\n",CurrentThread::tid(), str.c_str());
}

int main()
{
    cout << "main pid =" << ::getpid() << " tid = " << CurrentThread::tid() << endl;
    
    ThreadPool pool("my threadPoll test");
    pool.start(5);

    cout << "Adding" << endl;
    pool.addTask(UserThreadTask01);
    pool.addTask(UserThreadTask01);

    for (int i = 0; i < 100; ++i)
    {
        char buf[32];
        snprintf(buf, sizeof buf, "task %d", i);
        pool.addTask(std::bind(UserThreadTask02, string(buf)));
    }

    sleep(3);
    cout << "Done" << endl;
    
    return 0;
}
