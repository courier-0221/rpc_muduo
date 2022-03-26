#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <memory>
#include <sys/epoll.h>
#include <errno.h>
#include "Channel.h"
#include "Acceptor.h"
#include "TcpConnection.h"
#include "Common.h"
#include "ICommonCallback.h"
#include "EventLoopThreadPool.h"
#include <map>
#include <vector>

using namespace std;

class TcpServer : public IAcceptorCallback
{
public:
    TcpServer(EventLoop* pLoop, NetAddress addr);
    ~TcpServer() { cout << "~TcpServer()" << endl; }
    void start();                           //服务器开始工作（设置监听回调，新客户端连接做处理）
    void setCallback(IMuduoUser* pUser);    //新客户端连接上来时设置用户三个半事件回调给TcpConnection
    virtual void newConnection(int sockfd); //重写该方法，Acceptor类中调用
    void removeConnection(const TcpConnectionPtr& conn);      //客户端断开连接时的服务端处理方法，从map中删除，TcpConnetion类中调用
    void setThreadNum(int numThreads) { if(0 <= numThreads) _threadPool->setThreadNum(numThreads); }
private:
    struct epoll_event _events[MAX_EVENTS]; //事件集合--epoll wait返回的监听事件
    map<int, TcpConnectionPtr> _connections;//连接队列--已经连接上来的客户端对应的TcpConnection
    unique_ptr<Acceptor> _pAcceptor;        //接收管理器--处理新客户端连接
    EventLoop* _pLoop;                      //事件循环句柄
    IMuduoUser* _pUser;                     //用户回调参数，三个半事件
    std::shared_ptr<EventLoopThreadPool> _threadPool;   //多IO线程
};

#endif
