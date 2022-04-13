#include "echo.pb.h"

#include "EventLoop.h"
#include "RpcServer.h"

namespace echo
{

class EchoServiceImpl : public EchoService
{
public:
    virtual void Echo(::google::protobuf::RpcController* controller,
                    const ::echo::EchoRequest* request,
                    ::echo::EchoResponse* response,
                    ::google::protobuf::Closure* done)
    {
        response->set_payload(request->payload());
        done->Run();
    }
};

}  // namespace echo

int main()
{
    std::cout << "main pid = " << ::getpid() << " tid = " << CurrentThread::tid() << std::endl;
    EventLoop loop;
    NetAddress addr("127.0.0.1", 11111);
    echo::EchoServiceImpl impl;
    RpcServer server(&loop, addr);
    server.registerService(&impl);
    server.start();
    loop.loop();
    google::protobuf::ShutdownProtobufLibrary();
}

