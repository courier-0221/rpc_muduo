#include "Thread.h"
#include "CurrentThread.h"
#include "BlockingQueue.h"

#include <memory>
#include <string>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <functional>
using namespace std;

class Test
{
public:
    Test(int threadNum)
    :_threadNum(threadNum)
    {
        for (int i = 0; i < _threadNum; i++)
        {
            //Thread param - name
            char name[32];
            snprintf(name, sizeof name, "work thread %d", i);
            
            _threads.emplace_back(new Thread(std::bind(&Test::consumerTask, this), name));  //emplace_back 临时对象push时 减少对象拷贝和构造次数
        }

        for (auto &thread : _threads)
        {
            thread->start();
        }
    }

    void consumerTask()
    {
        cout<< "son tid = " << CurrentThread::tid() << " starting "<< endl;
        while (true)
        {
            auto cb = _queue.take();
            cb();
        }
    }

    void producerTask(const string& str, void* param)
    {
        int *pParam = static_cast<int*>(param);
        cout<< "son tid = " << CurrentThread::tid() << " str = " << str << " param = " << *pParam  << endl;
    }

    void mainThreadToPutTask(int times)
    {
        cout << "main thread all son threads started" << endl;
        
        for (int i = 0; i < times; ++i)
        {
            int param = i*10;
            char buf[32];
            snprintf(buf, sizeof buf, "work thread times = %d", i);
            string strbuf(buf);

            //加入任务唤醒线程
            _queue.put(std::bind(&Test::producerTask, this, strbuf, &param));
            cout << "main thread tid = " << CurrentThread::tid() << " _threads.size = " 
            << _threads.size() << " put into _queue i = " << i << endl;
            sleep(1);
        }
    }

private:
    typedef std::function<void()> funcCB;
    int _threadNum;
    BlockingQueue<funcCB> _queue;
    vector<std::unique_ptr<Thread>> _threads;
};

//测试move 右值引用
void testMove()
{
    BlockingQueue<std::unique_ptr<int>> queue;
    
    queue.put(std::unique_ptr<int>(new int(3)));
    std::unique_ptr<int> x = queue.take();
    cout << "testMove." << *x << endl;

    *x = 300;
    queue.put(std::move(x));
    std::unique_ptr<int> y = queue.take();
    cout << "testMove." << *y << endl;
}

int main()
{
    cout <<"main pid =" << ::getpid() << " tid = " << CurrentThread::tid() << endl;
    
    //测试move 右值引用
    //testMove();

    Test t(3);
    //等待所有线程全部初始化完成
    sleep(1);

    t.mainThreadToPutTask(10);

    return 0;
}
