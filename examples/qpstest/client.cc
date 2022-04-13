#include "echo.pb.h"

#include "TcpClient.h"
#include "RpcChannel.h"
#include "ICommonCallback.h"
#include "TcpConnection.h"
#include "EventLoop.h"
#include "CurrentThread.h"
#include "Common.h"
#include "TimeStamp.h"

#include <stdio.h>

static const int kRequests = 50000;

class RpcClient : noncopyable, public IMuduoUser
{
public:
    RpcClient(EventLoop* loop, const NetAddress& serverAddr)
      : _loop(loop),
        _client(loop, serverAddr),
        _channel(new RpcChannel),
        _stub(_channel.get()),
        _count(0)
    {
    }

    void connect()
    {
        _client.setCallback(this);
        _client.connect();
    }

    void loopSendRpcRequest()
    {
        if (0 == _count)
        {
            _start = Timestamp::now();
        }
        echo::EchoRequest request;
        request.set_payload("001010");
        echo::EchoResponse* response = new echo::EchoResponse;
        
        _stub.Echo(NULL, &request, response, NewCallback(this, &RpcClient::solved, response));
    }

    void onConnection(const TcpConnectionPtr conn) override
    {
        _channel->setConnection(conn);
        loopSendRpcRequest();
    }

    virtual void onMessage(TcpConnectionPtr conn, Buffer* pBuf) override
    {
        _channel->onMessage(conn, pBuf);
    }

    virtual void onWriteComplate(TcpConnectionPtr conn) override
    {}

    virtual void onClose(TcpConnectionPtr conn) override 
    {cout << "RpcClient::onClose" << endl;}

private:
    
    void solved(echo::EchoResponse* resp)
    {
        ++_count;
        if (_count < kRequests)
        {
            loopSendRpcRequest();
        }
        else
        {
            std::cout << "RpcClient " << this << " finished" << std::endl;
            _end = Timestamp::now();

            double seconds = timeDifference(_end, _start);
            printf("%f seconds\n", seconds);
            printf("%.1f calls per second\n", kRequests / seconds);
        }
    }

    EventLoop* _loop;
    TcpClient _client;
    RpcChannelPtr _channel;
    echo::EchoService::Stub _stub;
    int _count;
    Timestamp _start;
    Timestamp _end;
};

int main(int argc, char* argv[])
{
    std::cout << "main pid = " << ::getpid() << " tid = " << CurrentThread::tid() << std::endl;

    EventLoop loop;
    NetAddress addr("127.0.0.1", 11111);
    RpcClient rpcClient(&loop, addr);
    rpcClient.connect();
    loop.loop();

    google::protobuf::ShutdownProtobufLibrary();
}

