#include "sudoku.pb.h"

#include "TcpClient.h"
#include "RpcChannel.h"
#include "ICommonCallback.h"
#include "TcpConnection.h"
#include "EventLoop.h"
#include "CurrentThread.h"
#include "Common.h"

#include <stdio.h>

class RpcClient : noncopyable, public IMuduoUser
{
public:
    RpcClient(EventLoop* loop, const NetAddress& serverAddr)
      : _loop(loop),
        _client(loop, serverAddr),
        _channel(new RpcChannel),
        _stub(_channel.get())
    {
    }

    void connect()
    {
        _client.setCallback(this);
        _client.connect();
    }

    void loopSendRpcRequest()
    {
        sudoku::SudokuRequest request;
        request.set_checkerboard("001010");
        sudoku::SudokuResponse* response = new sudoku::SudokuResponse;
        
        _stub.Solve(NULL, &request, response, NewCallback(this, &RpcClient::solved, response));
    }

    void onConnection(const TcpConnectionPtr conn) override
    {
        _channel->setConnection(conn);
    }

    virtual void onMessage(TcpConnectionPtr conn, Buffer* pBuf) override
    {
        _channel->onMessage(conn, pBuf);
    }

    virtual void onWriteComplate(TcpConnectionPtr conn) override
    {cout << "RpcClient::onWriteComplate" << endl;}

    virtual void onClose(TcpConnectionPtr conn) override 
    {cout << "RpcClient::onClose" << endl;}

private:
    
    void solved(sudoku::SudokuResponse* resp)
    {
        std::cout << "solved:\n" << resp->DebugString() << std::endl;
    }

    EventLoop* _loop;
    TcpClient _client;
    RpcChannelPtr _channel;
    sudoku::SudokuService::Stub _stub;
};

int main(int argc, char* argv[])
{
    std::cout << "main pid = " << ::getpid() << " tid = " << CurrentThread::tid() << std::endl;

    EventLoop loop;
    NetAddress addr("127.0.0.1", 11111);
    RpcClient rpcClient(&loop, addr);
    rpcClient.connect();
    loop.runEvery(3, std::bind(&RpcClient::loopSendRpcRequest, &rpcClient));
    loop.loop();

    google::protobuf::ShutdownProtobufLibrary();
}

