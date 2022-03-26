#include "EchoServerMultCalcThread.h"

EchoServerMultCalcThread::EchoServerMultCalcThread(EventLoop* pLoop, NetAddress addr)
    :_pLoop(pLoop)
    ,_pServer(pLoop, addr)
{
    _pServer.setCallback(this);
}

EchoServerMultCalcThread::~EchoServerMultCalcThread()
{}

void EchoServerMultCalcThread::start()
{
    _pServer.start();
    _threadpool.start(3);
}

void EchoServerMultCalcThread::onConnection(TcpConnectionPtr conn)
{
    cout << "EchoServerMultCalcThread::onConnection" << endl;
}

void EchoServerMultCalcThread::onMessage(TcpConnectionPtr conn, Buffer* pBuf)
{
    string msg(pBuf->retrieveAllAsString());
    cout << "EchoServerMultCalcThread::onMessage" << " recv " << msg.size() << " bytes " << endl;
    _threadpool.addTask(std::bind(&EchoServerMultCalcThread::echo, this, msg, conn.get()));
}

void EchoServerMultCalcThread::onWriteComplate(TcpConnectionPtr conn)
{
    cout << "EchoServerMultCalcThread::onWriteComplate" << endl;
}

void EchoServerMultCalcThread::onClose(TcpConnectionPtr conn)
{
    cout << "EchoServerMultCalcThread::onClose" << endl;
}

//回显方法
void EchoServerMultCalcThread::echo(const string& str, void* param)
{
    cout << "EchoServerMultCalcThread::echo SEND" << " tid = " << CurrentThread::tid() << endl;
    TcpConnection* pTcpCon = static_cast<TcpConnection*>(param);
    pTcpCon->send(str + "\n");
}