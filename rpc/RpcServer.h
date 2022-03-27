#ifndef RPCSERVER_H
#define RPCSERVER_H

#include "TcpServer.h"
#include "RpcChannel.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>

class RpcServer : noncopyable, public IMuduoUser
{
public:
    RpcServer(EventLoop* loop, const NetAddress& listenAddr);
    void setThreadNum(int numThreads) { _server.setThreadNum(numThreads); }

    void registerService(google::protobuf::Service*);
    void start();

private:
    virtual void onConnection(TcpConnectionPtr conn) override;
    virtual void onMessage(TcpConnectionPtr conn, Buffer* pBuf) override;
    virtual void onWriteComplate(TcpConnectionPtr conn) override { std::cout << "RpcServer onWriteComplate" << std::endl;}
    virtual void onClose(TcpConnectionPtr conn) override { std::cout << "RpcServer onClose" << std::endl;}

    TcpServer _server;
    std::map<std::string, ::google::protobuf::Service*> _services;
};

#endif  // RPCSERVER_H
