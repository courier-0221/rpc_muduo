#include "Thread.h"
#include "CurrentThread.h"

class Foo
{
public:
	Foo(int count) : _count(count)
	{
	}

	void MemberFun()
	{
        cout <<"MemberFun() son pid = " << ::getpid() << " tid = " << CurrentThread::tid() << endl;
		while (_count--)
		{
			cout << "MemberFun() this is a test ..." << endl;
			sleep(1);
		}
	}

	void MemberFun2(int x)
	{
        cout <<"MemberFun2(int x) son pid = " << ::getpid() << " tid = " << CurrentThread::tid() << endl;
		while (_count--)
		{
			cout << "MemberFun2(int x) x = " << x << " this is a test2 ..." << endl;
			sleep(1);
		}
	}

	int _count;
};

void ThreadFunc()
{
	cout << "ThreadFunc() son pid = " << ::getpid() << " tid = " << CurrentThread::tid() << endl;
}

void ThreadFunc2(int count)
{
    cout << "ThreadFunc2(int count) son pid = " << ::getpid() << " tid = " << CurrentThread::tid() << endl;
	while (count--)
	{
		cout << "ThreadFunc2(int count) ..." << endl;
		sleep(1);
	}
}

int main()
{
    cout <<"main pid = " << ::getpid() << " tid = " << CurrentThread::tid() << endl;
    
    //绑定普通函数
    Thread t1(ThreadFunc);
	Thread t2(std::bind(ThreadFunc2, 3));

    //绑定类成员函数
	Foo foo(3);
	Thread t3(std::bind(&Foo::MemberFun, &foo));
	Foo foo2(3);
	Thread t4(std::bind(&Foo::MemberFun2, &foo2, 1000));

	t1.start();
	t2.start();
	t3.start();
	t4.start();

	t1.join();
	t2.join();
	t3.join();
	t4.join();

    return 0;
}