#include "Channel.h"

Channel::Channel(EventLoop* pLoop, int sockfd)
    :_sockfd(sockfd)
    ,_events(0)
    ,_revents(0)
    ,_index(kNew)
    ,_pCallback(NULL)
    ,_pLoop(pLoop)
{
}

void Channel::handleEvent()
{
    //读写都关闭。
    if ((_revents & EPOLLHUP) && !(_revents & EPOLLIN))
    {
        cout << "sockfd = " << _sockfd << "Channel::handleEvent() EPOLLHUP" << endl;
        if (_pCallback) _pCallback->handleClose();
    }
    //读事件
    if (_revents & EPOLLIN)
    {
        if (_pCallback) _pCallback->handleRead();
    }
    //写事件
    if (_revents & EPOLLOUT)
    {
        if (_pCallback) _pCallback->handleWrite();
    }
}

void Channel::update() 
{
    _pLoop->update(this);
}

string Channel::reventsToString() const
{
  return eventsToString(_sockfd, _revents);
}

string Channel::eventsToString() const
{
  return eventsToString(_sockfd, _events);
}

string Channel::eventsToString(int fd, int ev)
{
    std::ostringstream oss;
    oss << fd << ": ";
    if (ev & EPOLLIN)
        oss << "IN ";
    if (ev & EPOLLPRI)
        oss << "PRI ";
    if (ev & EPOLLOUT)
        oss << "OUT ";
    if (ev & EPOLLHUP)
        oss << "HUP ";
    if (ev & EPOLLRDHUP)
        oss << "RDHUP ";
    if (ev & EPOLLERR)
        oss << "ERR ";
    if (ev == 0)
        oss << "NULL";

    return oss.str();
}