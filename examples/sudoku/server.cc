#include "sudoku.pb.h"

#include "EventLoop.h"
#include "RpcServer.h"

namespace sudoku
{

class SudokuServiceImpl : public SudokuService
{
public:
    virtual void Solve(google::protobuf::RpcController* controller,
                          const ::sudoku::SudokuRequest* request,
                          ::sudoku::SudokuResponse* response,
                          google::protobuf::Closure* done)
    {
        std::cout << "SudokuServiceImpl::Solve" << std::endl;
        response->set_solved(true);
        response->set_checkerboard("1234567");
        done->Run();
    }
};

}  // namespace sudoku

int main()
{
    std::cout << "main pid = " << ::getpid() << " tid = " << CurrentThread::tid() << std::endl;
    EventLoop loop;
    NetAddress addr("127.0.0.1", 11111);
    sudoku::SudokuServiceImpl impl;
    RpcServer server(&loop, addr);
    server.registerService(&impl);
    server.start();
    loop.loop();
    google::protobuf::ShutdownProtobufLibrary();
}

