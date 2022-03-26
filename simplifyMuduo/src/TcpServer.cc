#include "TcpServer.h"

TcpServer::TcpServer(EventLoop* pLoop, NetAddress addr)
    : _pAcceptor(new Acceptor(pLoop, addr))
    , _pLoop(pLoop)
    , _pUser(NULL)
    , _threadPool(new EventLoopThreadPool(pLoop))
{
}

void TcpServer::start()
{
    _pAcceptor->setCallback(this);
    _pAcceptor->start();
    _threadPool->start();   //启动IO线程池
}

void TcpServer::newConnection(int sockfd)
{
    EventLoop* ioLoop = _threadPool->getNextLoop();
    TcpConnectionPtr conn(new TcpConnection(sockfd, ioLoop));
    _connections[sockfd] = conn;    //加入到连接管理中
    conn->setUser(_pUser);
    conn->setUpLayerCloseCallback(bind(&TcpServer::removeConnection, this, std::placeholders::_1)); //设置上层的连接断开回调给TcpConnection
    //conn->connectEstablished();     //连接建立-透传调用到用户设置的onConnection
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    _connections.erase(conn->getFd());  //从连接管理中删除
    cout << "TcpServer::removeConnection sockfd [" << conn->getFd() << "] " 
        << "TcpServer::_connections.size [" << _connections.size() << "] " << " tid = " << CurrentThread::tid() << endl;
}

void TcpServer::setCallback(IMuduoUser* user)
{
    _pUser = user;
}
