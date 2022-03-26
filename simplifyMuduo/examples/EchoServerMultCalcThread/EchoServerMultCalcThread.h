#ifndef ECHOSERVERMULTCALCTHREAD_H
#define ECHOSERVERMULTCALCTHREAD_H

#include "ICommonCallback.h"
#include "TcpServer.h"
#include "ThreadPool.h"
#include "Acceptor.h"
#include "TcpConnection.h"
#include "EventLoop.h"
#include "CurrentThread.h"

#include <iostream>
using namespace std;

class EchoServerMultCalcThread : public IMuduoUser
{
public:
    EchoServerMultCalcThread(EventLoop* pLoop, NetAddress addr);
    ~EchoServerMultCalcThread();
    void start();
    virtual void onConnection(TcpConnectionPtr conn);
    virtual void onMessage(TcpConnectionPtr conn, Buffer* pBuf);  //从buf中读 然后交给多计算线程处理
    virtual void onWriteComplate(TcpConnectionPtr conn);  //分配给多个线程任务
    virtual void onClose(TcpConnectionPtr conn);

    //回显方法 配合多计算线程调用sendInLoop方法发送
    void echo(const string& str, void* param);
private:
    EventLoop* _pLoop;
    TcpServer _pServer;
    ThreadPool _threadpool;
};

#endif
