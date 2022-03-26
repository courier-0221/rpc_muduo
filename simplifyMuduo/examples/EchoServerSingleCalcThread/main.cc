#include "EchoServerSingleCalcThread.h"

int main(int args, char** argv)
{
    cout << "main" << " tid = " << CurrentThread::tid() << endl;

    EventLoop loop;
    NetAddress addr("127.0.0.1", 11111);
    EchoServerSingleCalcThread echoserver(&loop, addr);
    echoserver.start();
    loop.loop();
    return 0;
}
