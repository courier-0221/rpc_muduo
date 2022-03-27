#include "RpcChannel.h"

RpcChannel::RpcChannel()
    : _codec(&RpcMeta::RpcMetaMessage::default_instance(), "RPC0", 
          std::bind(&RpcChannel::onRpcMessage, this, std::placeholders::_1, std::placeholders::_2)),
      _id(0),
      _services(NULL)
{
    std::cout << "RpcChannel::ctor - " << this << std::endl;;
}

RpcChannel::RpcChannel(const TcpConnectionPtr& conn)
    : _codec(&RpcMeta::RpcMetaMessage::default_instance(), "RPC0", 
          std::bind(&RpcChannel::onRpcMessage, this, std::placeholders::_1, std::placeholders::_2)),
      _conn(conn),
      _services(NULL)
{
    std::cout << "RpcChannel::ctor - " << this << std::endl;;
}

RpcChannel::~RpcChannel()
{
    std::cout << "RpcChannel::dtor - " << this << std::endl;;
    for (const auto& outstanding : _outstandings)
    {
        OutstandingCall out = outstanding.second;
        delete out.response;
        delete out.done;
    }
}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller,
                            const google::protobuf::Message* request,
                            google::protobuf::Message* response,
                            google::protobuf::Closure* done)
{
    RpcMeta::RpcMetaMessage message;
    message.set_type(RpcMeta::REQUEST);
    int64_t id = incrementAndGet();
    message.set_id(id);
    message.set_service(method->service()->full_name());
    message.set_method(method->name());
    message.set_request(request->SerializeAsString());

    OutstandingCall out = { response, done };
    {
        MutexLockGuard lock(_mutex);
        _outstandings[id] = out;
    }
    _codec.send(_conn, message);
}

void RpcChannel::onMessage(const TcpConnectionPtr& conn, Buffer* buf)
{
    _codec.onMessage(conn, buf);
}

void RpcChannel::onRpcMessage(const TcpConnectionPtr& conn, const RpcMetaMessagePtr& messagePtr)
{
    printf("%s\n", messagePtr->DebugString().c_str());
    RpcMeta::RpcMetaMessage& message = *messagePtr;
    //rpc 客户端业务
    if (message.type() == RpcMeta::RESPONSE)
    {
        int64_t id = message.id();

        OutstandingCall out = { NULL, NULL };

        {
            MutexLockGuard lock(_mutex);
            std::map<int64_t, OutstandingCall>::iterator it = _outstandings.find(id);
            if (it != _outstandings.end())
            {
                out = it->second;
                _outstandings.erase(it);
            }
        }

        if (out.response)
        {
            std::unique_ptr<google::protobuf::Message> d(out.response);
            if (message.has_response())
            {
                out.response->ParseFromString(message.response());
            }
            if (out.done)
            {
                out.done->Run();
            }
        }
    }
    //rpc 服务端业务
    else if (message.type() == RpcMeta::REQUEST)
    {
        RpcMeta::ErrorCode error = RpcMeta::WRONG_PROTO;
        if (_services)
        {
            std::map<std::string, google::protobuf::Service*>::const_iterator it = _services->find(message.service());
            if (it != _services->end())
            {
                google::protobuf::Service* service = it->second;
                assert(service != NULL);
                const google::protobuf::ServiceDescriptor* desc = service->GetDescriptor();
                const google::protobuf::MethodDescriptor* method
                  = desc->FindMethodByName(message.method());
                if (method)
                {
                    std::unique_ptr<google::protobuf::Message> request(service->GetRequestPrototype(method).New());
                    if (request->ParseFromString(message.request()))
                    {
                        google::protobuf::Message* response = service->GetResponsePrototype(method).New();
                        // response is deleted in doneCallback
                        int64_t id = message.id();
                        service->CallMethod(method, NULL, request.get(), response,
                                            NewCallback(this, &RpcChannel::doneCallback, response, id));
                        error = RpcMeta::NO_ERROR;
                    }
                    else
                    {
                        error = RpcMeta::INVALID_REQUEST;
                    }
                }
                else
                {
                    error = RpcMeta::NO_METHOD;
                }
            }
            else
            {
                error = RpcMeta::NO_SERVICE;
            }
        }
        else
        {
            error = RpcMeta::NO_SERVICE;
        }
        if (error != RpcMeta::NO_ERROR)
        {
            RpcMeta::RpcMetaMessage response;
            response.set_type(RpcMeta::RESPONSE);
            response.set_id(message.id());
            response.set_error(error);
            _codec.send(_conn, response);
        }
    }
    else if (message.type() == RpcMeta::ERROR)
    {
    }
}

void RpcChannel::doneCallback(google::protobuf::Message* response, int64_t id)
{
    std::unique_ptr<google::protobuf::Message> d(response);
    RpcMeta::RpcMetaMessage message;
    message.set_type(RpcMeta::RESPONSE);
    message.set_id(id);
    message.set_response(response->SerializeAsString());
    _codec.send(_conn, message);
}

