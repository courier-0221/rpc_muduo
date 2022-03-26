#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>

#include "Acceptor.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Common.h"
#include "ICommonCallback.h"

#include <iostream>
#include <string>
using namespace std;

//网络地址地址
class NetAddress
{
public:
    NetAddress(string ip, uint16_t port = 11111)
    {
        bzero(&_ipv4Addr, sizeof _ipv4Addr);
        _ipv4Addr.sin_family = AF_INET;
        if (::inet_pton(AF_INET, ip.c_str(), &_ipv4Addr.sin_addr) <= 0)
        {
            cout << "sockets::fromIpPort" << endl;
        }
        _ipv4Addr.sin_port = htons(port);
    }

    const struct sockaddr_in* getIpv4Addr() const { return &_ipv4Addr; }
    void setIpv4Addr(const struct sockaddr_in& addr4) { _ipv4Addr = addr4; }

private:
    struct sockaddr_in _ipv4Addr;
};

class Acceptor : public IChannelCallback
{
public:
    Acceptor(EventLoop* pLoop, NetAddress addr);
    ~Acceptor() { cout << "~Acceptor()" << endl; }

    void start();
    void setCallback(IAcceptorCallback* pCallback);
    virtual void handleRead();
    virtual void handleWrite() {}
    virtual void handleClose() {}
private:
    int createAndListen();
    EventLoop* _pLoop;
    NetAddress _serverAddr;     //server ip+port, only ipv4
    int _listenfd;
    Channel _acceptChannel;
    IAcceptorCallback* _pCallback;  //客户端连接时的API
};

#endif
