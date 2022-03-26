#ifndef ICOMMONCALLBACK_H
#define ICOMMONCALLBACK_H

#include "Common.h"
#include <string>
using namespace std;

class IChannelCallback
{
public:
    virtual void handleRead() = 0;
    virtual void handleWrite() = 0;
    virtual void handleClose() = 0;
};

class IAcceptorCallback
{
public:
    virtual void newConnection(int sockfd) = 0; //新客户端连接上服务器 服务端视角
};

class IConnectorCallback
{
public:
    virtual void newConnection(int sockfd) = 0; //客户端与服务器建立了连接 客户端视角
};

class IMuduoUser
{
public:
    virtual void onConnection(TcpConnectionPtr conn) = 0;
    virtual void onMessage(TcpConnectionPtr conn, Buffer* pBuf) = 0;
    virtual void onWriteComplate(TcpConnectionPtr conn) = 0;
    virtual void onClose(TcpConnectionPtr conn) = 0;
};

#endif