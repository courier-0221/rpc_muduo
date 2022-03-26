#include "Connector.h"

Connector::Connector(EventLoop* pLoop, NetAddress addr)
    : _pLoop(pLoop)
    , _serverAddr(addr)
    , _sockFd(-1)
    , _pCallback(NULL)
{
}

void Connector::stop()
{
    _pLoop->queueInLoop(std::bind(&Connector::resetChannel, this)); 
}

void Connector::start()
{
    _pLoop->runInLoop(std::bind(&Connector::connect, this));
}

void Connector::connect()
{
    _sockFd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (_sockFd < 0)
    {
        cout << "Connector::createAndConnect error" << endl;
    }
    struct sockaddr_in servaddr;
    memcpy(&servaddr, _serverAddr.getIpv4Addr(), sizeof(struct sockaddr_in));
    int ret = ::connect(_sockFd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in));
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
        {
            _pSockChan.reset(new Channel(_pLoop, _sockFd));
            _pSockChan->setCallback(this);
            _pSockChan->enableWriting();  //连接成功后监听是否可写
        }
        break;

        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
        //retry(_sockFd);
        break;

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
        {
            cout << "connect error in Connector::startInLoop " << savedErrno << endl;
            ::close(_sockFd);
        }
        break;

        default:
        {
            cout << "Unexpected error in Connector::startInLoop " << savedErrno << endl;
            ::close(_sockFd);
        }
        break;
    }
}

void Connector::resetChannel()
{
    _pSockChan.reset(); //销毁channel
}

void Connector::handleWrite()
{
    //重置channel，建立连接的socket可写就说明客户端和服务器建立了连接，此时用于两端建立连接的channel就失去了作用可以丢掉
    //再一次保证连接已经建立上
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    if (::getsockopt(_sockFd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        char t_errnobuf[512];
        cout << "Connector::handleWrite - SO_ERROR = " << errno << " " << strerror_r(errno, t_errnobuf, sizeof t_errnobuf) << endl;
        //retry(_sockFd);
    }
    else
    {
        _pSockChan->setIndex(kDeleted);
        _pSockChan->disableAll();
        // Can't reset channel_ here, because we are inside Channel::handleEvent
        _pLoop->queueInLoop(std::bind(&Connector::resetChannel, this)); //loop中异步调用
        _pCallback->newConnection(_sockFd);
    }
}