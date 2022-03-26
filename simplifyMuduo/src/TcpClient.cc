#include "TcpClient.h"

TcpClient::TcpClient(EventLoop* loop, const NetAddress& serverAddr)
    : _pLoop(loop)
    , _serverAddr(serverAddr)
    , _pUser(NULL)
    , _connector(new Connector(loop, serverAddr))
    , _connection(NULL)
{
}

TcpClient::~TcpClient()
{
}

void TcpClient::connect()
{
    _connector->setCallback(this);
    _connector->start();
}

void TcpClient::newConnection(int sockfd) //重写该方法，Connector类中调用
{
    cout << "TcpClient::connect new connecting to "
            << "[" << inet_ntoa(_serverAddr.getIpv4Addr()->sin_addr) 
            << ":" << ntohs(_serverAddr.getIpv4Addr()->sin_port) << "] "
            << "new socket fd [" << sockfd << "] " << endl;
    TcpConnectionPtr conn(new TcpConnection(sockfd, _pLoop));
    conn->setUser(_pUser);
    conn->connectEstablished();     //连接建立-透传调用到用户设置的onConnection
    conn->setUpLayerCloseCallback(std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));
    _connection = conn;
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn)      //客户端断开连接时的服务端处理方法
{
    if (conn == _connection)
    {
        _connection.reset();
    }
    else
    {
        cout << "TcpClient::removeConnection error not equal" << endl;
    }
}