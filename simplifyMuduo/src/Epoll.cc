#include "Epoll.h"

Epoll::Epoll()
{
    _epollfd = ::epoll_create(1);
    if (_epollfd <= 0)
    {
        cout << "epoll_create error, errno:" << _epollfd << endl;
    }
}

Epoll::~Epoll()
{ cout << "Epoll::~Epoll()" << endl; }

void Epoll::poll(vector<Channel*>* pOutChannels)
{
    int fds = ::epoll_wait(_epollfd, _events, MAX_EVENTS, -1);
    if (fds == -1)
    {
        cout << "epoll_wait error, errno:" << errno << endl;
        return;
    }
    for(int i = 0; i < fds; i++)
    {
        Channel* pNodeChannel = static_cast<Channel*>(_events[i].data.ptr);
        pNodeChannel->setRevents(_events[i].events);    //设置已经产生的事件
        pOutChannels->push_back(pNodeChannel);
    }
}

void Epoll::update(Channel* pChannel)
{
    struct epoll_event ev;
    ev.data.ptr = pChannel;
    ev.events = pChannel->getEvents();
    int fd = pChannel->getfd();
    int index = pChannel->getIndex();
    cout << "epoll_ctl op = " << operationToString(index)
        << " fd = " << fd << " event = { " << pChannel->eventsToString() << " }" << endl;
    
    if (index == kNew)
    {
        //index创建channel的默认值为kNew，会使用ctl进行ADD，后续使用setIndex将其改为kAdded，这样kAdded时不需要使用setIndex进行设置，后续设置kDeleted时要使用setIndex设置下
        pChannel->setIndex(kAdded);
        ::epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &ev);
    }
    else if (index == kAdded)
    {
        ::epoll_ctl(_epollfd, EPOLL_CTL_MOD, fd, &ev);
    }
    else if (index == kDeleted)
    {
        ::epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, &ev);
    }
}

const char* Epoll::operationToString(int op)
{
    switch (op)
    {
    case kNew:
        return "ADD";
    case kDeleted:
        return "DEL";
    case kAdded:
        return "MOD";
    default:
        return "Unknown Operation";
    }
}