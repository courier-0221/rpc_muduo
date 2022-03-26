#ifndef CONNECTOR_H
#define CONNECTOR_H

#include "Channel.h"
#include "Common.h"
#include "ICommonCallback.h"
#include "Acceptor.h"

#include <errno.h>
#include <string>
#include <memory>
using namespace std;

//类似于acceptor类的作用，这里只保证和服务器建立了连接，后续和服务器交互的读写内容交给TcpCoonection完成
//实现原理为使用一个Channel来监听和服务器建立连接的sockfd是不是可写的，如果是则创建一个TcpConnection并且connector中的Channel就是去了作用
class Connector : public IChannelCallback
{
public:
    Connector(EventLoop* pLoop, NetAddress addr);
    ~Connector() {cout << "Connector::~Connector()" << endl;}
    
    void start();
    void stop();
    
    virtual void handleRead() {}
    virtual void handleWrite();
    virtual void handleClose() {}

    void resetChannel();    //与服务器连接后该channel就不再需要了
    void setCallback(IConnectorCallback* pCallback) {_pCallback = pCallback;}    //设置与服务器连接成功时的回调函数

private:
    void connect();
    EventLoop* _pLoop;
    NetAddress _serverAddr;
    int _sockFd;
    IConnectorCallback* _pCallback;  //服务器连接成功时的回调函数
    std::unique_ptr<Channel> _pSockChan;
};

#endif
