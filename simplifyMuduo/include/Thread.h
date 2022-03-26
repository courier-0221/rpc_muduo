#ifndef THREAD_H
#define THREAD_H

#include "Common.h"

#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <functional>
#include <iostream>
using namespace std;

class Thread : noncopyable
{
public:
    typedef std::function<void ()> ThreadFunc;
    explicit Thread(const ThreadFunc& func, const string& name = string());
    ~Thread();
    void start();
    int join();
    pid_t gettid() { return static_cast<pid_t>(::syscall(SYS_gettid)); }
    void setAutoDelete(bool autoDelete) {_autoDelete = autoDelete;}
    static void* ThreadRoutine(void* arg);
private:
    void Run();
    void setDefaultName();
    bool _started;  //线程是否创建开始执行
    bool _joined;   //调用join时析构中将不在detach
    bool _autoDelete;
    ThreadFunc _func;
    string _name;
    pthread_t _pthreadId;
};

#endif
