#include "Acceptor.h"

Acceptor::Acceptor(EventLoop* pLoop, NetAddress addr)
    :_pLoop(pLoop)
    ,_serverAddr(addr)
    ,_listenfd(createAndListen())
    ,_acceptChannel(pLoop, _listenfd)
    ,_pCallback(NULL)
{}

void Acceptor::start()
{
    _acceptChannel.setCallback(this);
    _acceptChannel.enableReading();
}

int Acceptor::createAndListen()
{
    int on = 1;
    _listenfd = ::socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    ::fcntl(_listenfd, F_SETFL, O_NONBLOCK); //no-block io
    ::setsockopt(_listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memcpy(&servaddr, _serverAddr.getIpv4Addr(), sizeof(struct sockaddr_in));

    if (-1 == ::bind(_listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)))
    {
        perror("bind error");
        exit(-1);
    }

    if (-1 == ::listen(_listenfd, MAX_LISTENFD))
    {
        perror("listen error");
        exit(-1);
    }
    return _listenfd;
}

void Acceptor::handleRead()
{
    int connfd;
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(struct sockaddr_in);
    connfd = accept(_listenfd, (sockaddr*)&cliaddr, (socklen_t*)&clilen);
    if (connfd > 0)
    {
        cout << "Acceptor::handleRead new connection from "
            << "[" << inet_ntoa(cliaddr.sin_addr) 
            << ":" << ntohs(cliaddr.sin_port) << "] "
            << "new socket fd [" << connfd << "] " << endl;
    }
    else
    {
        cout << "Acceptor::handleRead accept error, connfd:" << connfd << " errno:" << errno << endl;
    }
    fcntl(connfd, F_SETFL, O_NONBLOCK); //no-block io

    _pCallback->newConnection(connfd);
}

void Acceptor::setCallback(IAcceptorCallback* pCallback)
{
    _pCallback = pCallback;
}
