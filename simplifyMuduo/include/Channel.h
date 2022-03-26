//author voidccc
#ifndef CHANNEL_H
#define CHANNEL_H

#include <sys/epoll.h>
#include "Common.h"
#include "ICommonCallback.h"
#include "EventLoop.h"

#include <iostream>
#include <sstream>
using namespace std;

class Channel
{
public:
    Channel(EventLoop* pLoop, int sockfd);
    ~Channel() { cout << "~Channel()" << endl; }
    void setCallback(IChannelCallback* pCallback) { _pCallback = pCallback; }
    void handleEvent();
    void setRevents(int revent) { _revents = revent; }
    void setIndex(int index) { _index = index; };
    int getIndex() { return _index; };
    
    void enableReading() { _events |= EPOLLIN; update(); }
    void disableReading() { _events &= ~EPOLLIN; update(); }
    void enableWriting() { _events |= EPOLLOUT; update(); }
    void disableWriting() { _events &= ~EPOLLOUT; update(); }
    void disableAll() { _events = 0; update(); }
    int getEvents() { return _events; }
    
    int getfd() { return _sockfd; }
    bool isWriting() { return _events & EPOLLOUT; }
    bool isReading() { return _events & EPOLLIN; }

    // for debug
    string reventsToString() const;
    string eventsToString() const;
    
private:
    static string eventsToString(int fd, int ev);
    void update();
    int _sockfd;
    int _events;    //要关注的事件，EPOLLIN，EPOLLOUT，EPOLLHUP等
    int _revents;   //发生的事件，epoll_wait中将产生的事件告知该字段
    int _index;     //事件操作符，kNew，kAdded，kDeleted，对应EPOLL_CTL_ADD，MOD，DEL。构造时默认值为kNew，配合设置_events字段时使用。
    IChannelCallback* _pCallback;   //事件产生的回调
    EventLoop* _pLoop;
};

#endif
