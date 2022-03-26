#include "TcpConnection.h"

TcpConnection::TcpConnection(int sockfd, EventLoop* pLoop)
    :_sockfd(sockfd)
    ,_pLoop(pLoop)
    ,_pUser(NULL)
    ,_pSocketChannel(new Channel(_pLoop, _sockfd))
{
    _pSocketChannel->setCallback(this);
    _pSocketChannel->enableReading();
}

TcpConnection::~TcpConnection()
{ cout << "~TcpConnection()" << endl; }

void TcpConnection::handleRead()
{
    int sockfd = _pSocketChannel->getfd();
    int savedErrno = 0;
    
    if (sockfd < 0)
    {
        cout << "EPOLLIN sockfd < 0 error " << endl;
        return;
    }
    
    ssize_t readlength = _inBuf.readFd(sockfd, &savedErrno);
    if (readlength > 0)
    {
        _pUser->onMessage(shared_from_this(), &_inBuf);
    }
    else if (readlength == 0)
    {
        cout << "read 0 closed socket fd:" << sockfd << endl;
        handleClose();
    }
    else
    {
        if (savedErrno == ECONNRESET)
        {
            cout << "ECONNREST closed socket fd:" << sockfd << endl;
            handleClose();
        }
    }
}

void TcpConnection::handleWrite()
{
    int sockfd = _pSocketChannel->getfd();
    if (_pSocketChannel->isWriting())
    {
        int n = ::write(sockfd, _outBuf.peek(), _outBuf.readableBytes());
        if ( n > 0)
        {
            cout << "write " << n << " bytes data again" << endl;
            _outBuf.retrieve(n);
            if (_outBuf.readableBytes() == 0)
            {
                _pSocketChannel->disableWriting(); //从epoll中移除 EPOLLOUT
                _pLoop->queueInLoop(std::bind(&TcpConnection::onWriteComplateCB, this)); //调用 onWriteComplate
                //void (TcpConnection::*fp)(void) = &TcpConnection::onWriteComplateCB;
                //_pLoop->queueInLoop(std::bind(fp, this));
            }
        }
    }
}

void TcpConnection::handleClose()
{
    //设置事件监听操作为删除
    _pSocketChannel->setIndex(kDeleted);
    //设置事件监听类型为0，该fd已经断开连接，关闭该sockfd在epoll中监听的所有事件
    _pSocketChannel->disableAll();
    //调用用户设置的连接断开API
    TcpConnectionPtr guardThis(shared_from_this());
    if (_pUser) _pUser->onClose(guardThis);
    //调用TcpServer或者TcpClient的断开连接回调，用来做相应清除
    if (_closeCallback) _closeCallback(guardThis);
}

 //决策发送 如果在loop线程则立即发送，否则将任务加入到EventLoop的异步队列中
void TcpConnection::send(const string& message)
{
    if (_pLoop->isInLoopThread())
    {
        sendInLoop(message);    //立即发送
    }
    else
    {
        _pLoop->runInLoop(std::bind(&TcpConnection::onMessageCB, this, message)); //异步发送
    }
}

//立即发送 如果调用处为非loop线程也可以发送
void TcpConnection::sendInLoop(const string& message)
{
    int n = 0;
    if (_outBuf.readableBytes() == 0)
    {
        n = ::write(_sockfd, message.c_str(), message.size());
        if (n < 0)
            cout << "write error" << endl;
        if (n == static_cast<int>(message.size()))
        {
            _outBuf.retrieveAll();
            _pLoop->queueInLoop(std::bind(&TcpConnection::onWriteComplateCB, this)); //调用 onWriteComplate
        }
    }

    //没写完继续写
    if ( n < static_cast<int>(message.size()))
    {
        _outBuf.append(message.substr(n, message.size()));
        if (!_pSocketChannel->isWriting())
        {
            _pSocketChannel->enableWriting(); //add EPOLLOUT
        }
    }
}

void TcpConnection::connectEstablished()
{
    if (_pUser) _pUser->onConnection(shared_from_this());
}

void TcpConnection::setUser(IMuduoUser* user)
{
    if (user) _pUser = user;
}

void TcpConnection::onWriteComplateCB()
{
    if (_pUser) _pUser->onWriteComplate(shared_from_this());
}

void TcpConnection::onMessageCB(const string& message)
{
    sendInLoop(message);
    //send(message);    两者的区别为前者不需要管是不是在loop线程即可发送数据，
    //后者会将 onMessageCB 任务转到loop线程中调用，流程多中转一步，所以不在这里调用它
}
