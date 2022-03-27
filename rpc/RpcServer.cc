#include "RpcServer.h"

RpcServer::RpcServer(EventLoop* loop, const NetAddress& listenAddr)
  : _server(loop, listenAddr)
{
}

void RpcServer::registerService(google::protobuf::Service* service)
{
    const google::protobuf::ServiceDescriptor* desc = service->GetDescriptor();
    _services[desc->full_name()] = service;
    std::cout << "full_name: " << desc->full_name() << std::endl;
}

void RpcServer::start()
{
    _server.setCallback(this);
    _server.start();
}

void RpcServer::onConnection(const TcpConnectionPtr conn)
{
    std::cout << "RpcServer::onConnection new channel" << std::endl;

    RpcChannelPtr channel(new RpcChannel(conn));
    channel->setServices(&_services);
    conn->setContext(channel);
}

void RpcServer::onMessage(const TcpConnectionPtr conn, Buffer* buf)
{
    RpcChannelPtr channel = boost::any_cast<RpcChannelPtr>(conn->getContext());
    channel->onMessage(conn, buf);
}