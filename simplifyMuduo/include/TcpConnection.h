#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <string>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "Common.h"
#include "ICommonCallback.h"
#include "Buffer.h"
#include "Channel.h"
#include "EventLoop.h"

#include <boost/any.hpp>

#include <string.h>
#include <iostream>
#include <memory>
using namespace std;

typedef function<void (TcpConnectionPtr)> CloseCallbackForUpLayer;

//一个TcpConnection和一个channel绑定
class TcpConnection : noncopyable, public IChannelCallback, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(int sockfd, EventLoop* pLoop);
    ~TcpConnection();
    void send(const string& message);   //决策发送 如果在loop线程则立即发送，否则将任务加入到EventLoop的异步队列中
    void connectEstablished();  //连接建立处调用 -用户设置的回调
    void setUser(IMuduoUser* pUser);    //用户三个半事件回调函数
    void setUpLayerCloseCallback(const CloseCallbackForUpLayer& cb){ _closeCallback = cb; }
    int getFd() {return _sockfd;}

    void setContext(const boost::any& context) { _context = context; }
    const boost::any& getContext() const { return _context; }

    void onWriteComplateCB();   //调用用户设置的写完成cb
    void onMessageCB(const string& message); //调用用户设置的发送用户数据cb

    virtual void handleRead();
    virtual void handleWrite();
    virtual void handleClose();
    
private:
    void sendInLoop(const string& message);//立即发送
    int _sockfd;
    EventLoop* _pLoop;
    IMuduoUser* _pUser;
    unique_ptr<Channel> _pSocketChannel;
    CloseCallbackForUpLayer _closeCallback;   //断开连接时回调TcpServer或者TcpClient方法，调用处为handleClose
    Buffer _inBuf;
    Buffer _outBuf;
    boost::any _context;
};

#endif
