#include "EchoServerSingleCalcThread.h"

EchoServerSingleCalcThread::EchoServerSingleCalcThread(EventLoop* pLoop, NetAddress addr)
    :_pLoop(pLoop)
    ,_pServer(pLoop, addr)
{
    _pServer.setCallback(this);
}

EchoServerSingleCalcThread::~EchoServerSingleCalcThread()
{}

void EchoServerSingleCalcThread::start()
{
    _pServer.start();
}

void EchoServerSingleCalcThread::onConnection(TcpConnectionPtr conn)
{
    cout << "EchoServerSingleCalcThread::onConnection" << endl;
}

void EchoServerSingleCalcThread::onMessage(TcpConnectionPtr conn, Buffer* pBuf)
{
    string msg(pBuf->retrieveAllAsString());
    cout << "EchoServerSingleCalcThread::onMessage" << " recv " << msg.size() << " bytes " 
        << "SEND tid = " << CurrentThread::tid() << endl;
    conn->send(msg + "\n");
}

void EchoServerSingleCalcThread::onWriteComplate(TcpConnectionPtr conn)
{
    cout << "EchoServerSingleCalcThread::onWriteComplate" << endl;
}

void EchoServerSingleCalcThread::onClose(TcpConnectionPtr conn)
{
    cout << "EchoServerSingleCalcThread::onClose" << endl;
}