#include "CurrentThread.h"
#include "ICommonCallback.h"
#include "EventLoop.h"
#include "TimerQueue.h"
#include "Timestamp.h"
#include "Timer.h"

#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <stdio.h>
using namespace std;

int cnt = 0;
EventLoop* g_loop;

void print(const char* msg)
{
    cout << "msg: " << msg << " time: " << Timestamp::now().toString().c_str() << endl;
    if (++cnt == 20)
    {
        g_loop->quit();
    }
}

void cancel(TimerId timer)
{
    g_loop->cancelTimer(timer);
    cout << "cancelled at " << Timestamp::now().toString().c_str() << endl;
}

int main()
{
    cout << "main pid = " << ::getpid() << " tid = " << CurrentThread::tid() 
        << " now: " << Timestamp::now().toString().c_str() << endl;

    EventLoop loop;
    g_loop = &loop;

    print("main");
    loop.runAfter(1, std::bind(print, "once1"));
    loop.runAfter(1.5, std::bind(print, "once1.5"));
    loop.runAfter(2.5, std::bind(print, "once2.5"));
    loop.runAfter(3.5, std::bind(print, "once3.5"));
    loop.runEvery(2, std::bind(print, "every2"));
    TimerId t45 = loop.runAfter(4.5, std::bind(print, "once4.5"));
    TimerId t3 = loop.runEvery(3, std::bind(print, "every3"));
    
    loop.runAfter(4.2, std::bind(cancel, t45));
    loop.runAfter(4.8, std::bind(cancel, t45));
    loop.runAfter(5, std::bind(cancel, t3));

    loop.loop();
    print("main loop exits");

    return 0;
}
