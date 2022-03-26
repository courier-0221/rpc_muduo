#ifndef EPOLL_H
#define EPOLL_H

#include "Channel.h"
#include "Common.h"

#include <sys/epoll.h>
#include <errno.h>
#include <vector>
#include <iostream>
using namespace std;

class Epoll
{
public:
    Epoll();
    ~Epoll();
    void poll(vector<Channel*>* pChannels);
    void update(Channel* pChannel);
    static const char* operationToString(int op);
private:
    int _epollfd;
    struct epoll_event _events[MAX_EVENTS];
};

#endif
