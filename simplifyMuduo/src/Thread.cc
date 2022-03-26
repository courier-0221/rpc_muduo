#include "Thread.h"

namespace CurrentThread
{
    __thread int t_cachedTid = 0;
}

Thread::Thread(const ThreadFunc& func, const string& name)
    :_started(false)
    ,_joined(false)
    ,_autoDelete(false)
    ,_func(func)
    ,_name(name)
    ,_pthreadId(0)
{
    setDefaultName();
}

Thread::~Thread()
{
    if (_started && !_joined)
    {
        pthread_detach(_pthreadId); 
        cout << "~Thread()" << endl;
    }
}

int Thread::join()
{
    _joined = true; //调用join时 线程自动的析构中则不会再调用detach
    return pthread_join(_pthreadId, NULL);
}

void Thread::start()
{
    _started = true;
    if (0 != ::pthread_create(&_pthreadId, NULL, ThreadRoutine, this))
    {
        cout << "Thread::start pthread_create error " << endl;
        _started = false;
    }
}

void* Thread::ThreadRoutine(void* arg)
{
	Thread* thread = static_cast<Thread*>(arg);
    prctl(PR_SET_NAME, thread->_name);   //设置线程名字
	thread->Run();
	if ((thread->_autoDelete) && (thread))
    {
        cout << "Thread::ThreadRoutine _autoDelete = " << thread->_autoDelete << " delete Thread info " << endl;
        delete thread;
    }
    return NULL;
}

void Thread::Run()
{
	_func();
}

void Thread::setDefaultName()
{
    if (_name.empty())
    {
        char buf[32];
        snprintf(buf, sizeof buf, "Thread_%d", gettid());
        _name = buf;
    }
}