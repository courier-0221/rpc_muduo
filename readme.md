# muduo实现rpc

## 1.proto定义rpc方法介绍

```cpp
syntax = "proto2";

package sudoku;

message SudokuRequest {
  required string checkerboard = 1;
}

message SudokuResponse {
  optional bool solved = 1 [default=false];
  optional string checkerboard = 2;
}

service SudokuService {
  rpc Solve (SudokuRequest) returns (SudokuResponse);
}
```

protoc会对其中service SudokuService 这一句会生成SudokuService SudokuService _Stub两个类，分别是 server 端和 client 端需要关心的。

### 1.1.对 server 端

​	通过EchoService::Echo来处理请求，代码未实现，需要子类来 override.

```cpp
class SudokuService : public ::PROTOBUF_NAMESPACE_ID::Service {
    virtual void Solve(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sudoku::SudokuRequest* request,
                       ::sudoku::SudokuResponse* response,
                       ::google::protobuf::Closure* done);
}
void SudokuService::Solve(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                         const ::sudoku::SudokuRequest*,
                         ::sudoku::SudokuResponse*,
                         ::google::protobuf::Closure* done) {
  controller->SetFailed("Method Solve() not implemented.");
  done->Run();
}
```

### 1.2.对 client 端

​	通过EchoService_Stub来发送数据，EchoService_Stub::Echo调用了::google::protobuf::Channel::CallMethod，但是Channel是一个纯虚类，需要 RPC 框架在子类里实现需要的功能。

```cpp
class SudokuService_Stub : public SudokuService {
 void Solve(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sudoku::SudokuRequest* request,
                       ::sudoku::SudokuResponse* response,
                       ::google::protobuf::Closure* done);
}
void SudokuService_Stub::Solve(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                              const ::sudoku::SudokuRequest* request,
                              ::sudoku::SudokuResponse* response,
                              ::google::protobuf::Closure* done) {
  channel_->CallMethod(descriptor()->method(0),
                       controller, request, response, done);
}
```

## 2.server&client代码实现

### 2.1.server

继承SudokuService，override其中的 Solve 方法，业务逻辑为处理sudoku的请求处理；然后再将该service加入到<ServiceDescriptorName, Service*>中进行存储。

### 2.2.client

使用SudokuService_Stub ，该类型的实例化对象需要传入一个RpcChannel，需要Rpc框架来实现继承该类并重写，功能是实现数据的发送和接收。

### 2.3.疑问

#### 问题1

为什么 server 端只需要实现SudokuService::Slove函数，client端只需要调用SudokuService_Stub::Slove就能发送和接收对应格式的数据？中间的调用流程是怎样的？

#### 问题2

如果 server 端接收多种 pb 数据（例如还有一个 method rpc Post(DeepLinkReq) returns (DeepLinkResp);），那么怎么区分接收到的是哪个格式？

#### 问题3

区分之后，又如何构造出对应的对象来？例如SudokuService::Slove参数里的SudokuRequest SudokuResponse ，因为 rpc 框架并不清楚这些具体类和函数的存在，框架并不清楚具体类的名字，也不清楚 method 名字，却要能够构造对象并调用这个函数？

## 3.处理流程

### 3.1.server端处理流程

a.从对端接收数据
b.通过 **标识机制** 判断如何反序列化到 request 数据类型
c.生成对应的 response 数据类型
d.调用对应的 service-method ，填充 response 数据
e.序列化 response
f.发送数据回对端

接口设计上

​    Service是一个抽象类，CallMethod抽象方法，sudoku.proto会通过proroc工具生成 class SudokuService ，override其中的CallMethod方法，(逻辑是根据method->index()来决定调用哪个rpc方法)，调用rpc方法时将request或response通过down_cast统一为Message*格式，这样就解决了框架的接口问题。

### 3.2.client端处理流程

a.填充 request 数据调用对应的 service-method
b.发送数据到对端
c.从对端接收数据
d.通过 **回调机制** 判断如何反序列化到 response 数据类型
e.反序列化生成对应的 response 数据类型

SudokuService_Stub 继承自SudokuService ，其中Stub override 了其中的Slove方法，调用了Channel中的CallMethod方法。

**RpcChannel纯虚类**

::google::protobuf::RpcChannel 是一个纯虚类，类中定义了 CallMethod 抽象方法，可以将此类理解成一个通道连接了客户端和服务器端，本质上通过socket通信。所以需要基于RpcChannel开发一个子类实现CallMethod方法，实现的功能有两个:

a.序列化 request ，发送到对端，同时需要标识机制使得对端知道如何解析(schema)和处理(method)这类数据。

b.接收对端数据，反序列化到 response

## 4.标识机制

当服务端收到客户端发送的请求数据时，server端需要知道这段buffer对应的数据格式，应该调用哪个rpc方法处理，对应的返回格式是什么。设想一种这样的情况，一个service中用户定义了两个rpc方法，两个方法的请求参数和返回值都是一样的，这会服务端可以使用service name+method name来进行区分知道要调用哪个rpc方法来处理返回的response类型是什么样的，client请求时要带着这些内容。

pb 里有很多 xxxDescriptor 的类，service method也不例外。

例如

通过GetDescriptor可以获取ServiceDescriptor.

```cpp
class LIBPROTOBUF_EXPORT Service {
  ...

  // Get the ServiceDescriptor describing this service and its methods.
  virtual const ServiceDescriptor* GetDescriptor() = 0;
};//Service
```

通过ServiceDescriptor就可以获取对应的name及MethodDescriptor.

```cpp
class LIBPROTOBUF_EXPORT ServiceDescriptor {
 public:
  // The name of the service, not including its containing scope.
  const string& name() const;
  ...
  // The number of methods this service defines.
  int method_count() const;
  // Gets a MethodDescriptor by index, where 0 <= index < method_count().
  // These are returned in the order they were defined in the .proto file.
  const MethodDescriptor* method(int index) const;
};//ServiceDescriptor
```


而MethodDecriptor可以获取对应的name及从属的ServiceDescriptor

```cpp
class LIBPROTOBUF_EXPORT MethodDescriptor {
 public:
  // Name of this method, not including containing scope.
  const string& name() const;
  ...
  // Gets the service to which this method belongs.  Never NULL.
  const ServiceDescriptor* service() const;
};//MethodDescriptor
```

因此：

1. server 端传入一个`::google::protobuf::Service`时，我们可以记录 service name 及对应的Service*.
2. client 端调用`virtual void CallMethod(const MethodDescriptor* method...`时，也可以获取到 method name 及对应的 service name.

这样，就可以知道发送的数据类型了。

## 5.构造参数

实现 RPC 框架时，肯定是不知道`SudokuRequest SudokuResponse`类名的，但是通过`::google::protobuf::Service`的接口可以构造出对应的对象来

```cpp
  virtual const Message& GetRequestPrototype(
    const MethodDescriptor* method) const = 0;
  virtual const Message& GetResponsePrototype(
    const MethodDescriptor* method) const = 0;
```

而`Message`通过`New`可以构造出对应的对象

```cpp
class LIBPROTOBUF_EXPORT Message : public MessageLite {
 public:
  virtual Message* New() const = 0;
  ...
```

## 6.模块设计

### 6.1.RpcMeta

```cpp
syntax = "proto2";
package RpcMeta;
enum MessageType
{
  REQUEST = 1;
  RESPONSE = 2;
  ERROR = 3; 
}
message RpcMetaMessage
{
  required MessageType type = 1;
  required fixed64 id = 2;      //客户端每一次发起请求的消息id，服务端处理完成后再将id带回来

  optional string service = 3;
  optional string method = 4;
  optional bytes request = 5;

  optional bytes response = 6;

  optional ErrorCode error = 7;
}
```

每一次请求与回复中都带着该内容service和method标识对应name。

### 6.2.TransferHeader

```cpp
// Field     Length  Content
//
// size      4-byte  M+N+4
// tag       M-byte  could be "RPC0", etc.
// payload   N-byte
// checksum  4-byte  adler32 of tag+payload
```

对应C语言结构伪代码描述

```c
struct ProtobufTransportFormat
{
    int32_t size;
    char	tag[M];
    char	payload[N]
    int32_t checksum;	//adler32 of tag and payload
}
```

每一次传输中包裹上头部信息，payload位置处指定 RpcMetaMessage 内容。

### 6.3.ProtobufCodecLite

该类的作用是对发送和接收到的数据进行编解码操作，也就是对接收到的数据按照TransferHeader 结构进行拆解，对要发送的数据按照TransferHeader 结构进行填充。

```cpp
ProtobufCodecLite.h
class ProtobufCodecLite : noncopyable
{
public:
    ProtobufCodecLite(const RpcMeta::RpcMetaMessage* prototype,
                      std::string tagArg,
                      const ProtobufMessageCallback& messageCb,
                      const ErrorCallback& errorCb = defaultErrorCallback);

    void send(const TcpConnectionPtr& conn, const RpcMeta::RpcMetaMessage& message);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf);

    //序列化与反序列化
    bool parseFromBuffer(std::string buf, google::protobuf::Message* message);
    int serializeToBuffer(const google::protobuf::Message& message, Buffer* buf);

    ErrorCode parse(const char* buf, int len, RpcMeta::RpcMetaMessage* message);
    void fillEmptyBuffer(Buffer* buf, const google::protobuf::Message& message);
private:
    const RpcMeta::RpcMetaMessage* _prototype;
    ProtobufMessageCallback _messageCallback;
};
```

```cpp
ProtobufCodecLite.cc
void ProtobufCodecLite::fillEmptyBuffer(Buffer* buf, 
                                        const google::protobuf::Message& message)
{
    // size+tag+payload+checksum
    buf->append(_tag);  //tag

    int byteSize = serializeToBuffer(message, buf); //payload
    
    int32_t checkSum = checksum(buf->peek(), static_cast<int>(buf->readableBytes()));
    checkSum = hostToNetwork32(checkSum);
    buf->append(static_cast<const char*>((void*)&checkSum), sizeof checkSum);  //checksum
    
    int32_t len = hostToNetwork32(static_cast<int32_t>(buf->readableBytes()));
    buf->prepend(&len, sizeof len); //size
}
void ProtobufCodecLite::onMessage(const TcpConnectionPtr& conn, Buffer* buf)
{
    1.利用 网络库 收到一条完成消息
    2.利用protobuf构建rpcmeta内存
    3.解析TransferHeader 结构
    4.调用ProtobufCodecLite构造时传入的rpcmessage回调函数
}
void send(const TcpConnectionPtr& conn, const RpcMeta::RpcMetaMessage& message)
{
    1.fillEmptyBuffer()填充TransferHeader 结构
    2.利用 网络库 发送消息
}
```

### 6.4.RpcChannel

通过实现`Channel::CallMethod`方法，我们就可以在调用子类方法，例如`SudokuService_Stub::Slove`时自动实现数据的发送/接收、序列化/反序列化了。

```cpp
RpcChannel.h
class RpcChannel : public ::google::protobuf::RpcChannel
{
public:
    explicit RpcChannel(const TcpConnectionPtr& conn);
    void setServices(const std::map<std::string, ::google::protobuf::Service*>* services)
    {
        _services = services;
    }
    void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                    ::google::protobuf::RpcController* controller,
                    const ::google::protobuf::Message* request,
                    ::google::protobuf::Message* response,
                    ::google::protobuf::Closure* done) override;

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf);

private:
    void onRpcMessage(const TcpConnectionPtr& conn, const RpcMetaMessagePtr& messagePtr);
    void doneCallback(::google::protobuf::Message* response, int64_t id);

    struct OutstandingCall
    {
        ::google::protobuf::Message* response;
        ::google::protobuf::Closure* done;
    };

    ProtobufCodecLite _codec;
    TcpConnectionPtr _conn;
    std::map<int64_t, OutstandingCall> _outstandings;
    const std::map<std::string, ::google::protobuf::Service*>* _services;
};
```

```cpp
RpcChannel.cc
RpcChannel::RpcChannel(const TcpConnectionPtr& conn)
    : _codec(&RpcMeta::RpcMetaMessage::default_instance(), "RPC0", 
          std::bind(&RpcChannel::onRpcMessage, this, std::placeholders::_1, std::placeholders::_2)),
      _conn(conn),
      _services(NULL)
{
	//构造函数中初始化 ProtobufCodecLite 传入一个解析rpcmeta数据的回调函数
}
void RpcChannel::CallMethod(const ::google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller,
                            const ::google::protobuf::Message* request,
                            ::google::protobuf::Message* response,
                            ::google::protobuf::Closure* done)
{
    1.构造rpcmeta Message格式，messagetype为REQUEST，因为是客户端调用该方法进行服务请求
    2.构造OutstandingCall结构存储进 _outstandings 中，key为当前请求的id编号，value为
        OutstandingCall out = { response, done };，服务端处理完成发送该id编号回来，
        同样根据编号进行查找，拿到response调用用户的done接口。
    3.利用 网络库 发送消息
}

void RpcChannel::onMessage(const TcpConnectionPtr& conn, Buffer* buf)
{
    //利用 ProtobufCodecLite 接收消息
}

void RpcChannel::onRpcMessage(const TcpConnectionPtr& conn, 
                              const RpcMetaMessagePtr& messagePtr)
{
    ProtobufCodecLite接收到数据后 按照 TransferHeader 结构解码，将解析出的rpcmeta数据传递给该回调进行处理
    1.根据类型判定是REQUEST还是RESPONSE
    2.如果是RESPONSE	//client端处理业务
        2.1.rpcmeta中摘出id从 _outstandings 记录中查找，找到存储到临时变量然后进行erase删除
        2.2.rpcmeta中摘出response，反序列化
        2.3.调用用户的done接口
    3.如果是REQUEST	//server端处理业务
        3.1.rpcmeta中摘出service从 _services 结构中查找对应的 Service 服务
        3.2.rpcmeta中摘出method从 Service 服务 中查找对应的 method
        3.3.根据method 构造 request 内存，同时反序列化
        3.4.根据method 构造 respinse 内存，调用service的CallMethod方法，执行客户override
        	的rpc方法,执行完成后服务完成回调 doneCallback
}

void RpcChannel::doneCallback(::google::protobuf::Message* response, int64_t id)
{
    1.销毁response内存
    2.构造rpcmeta中相应内容
    3.利用 网络库 发送response给客户端
}

```

### 6.5.RpcServer

```cpp
RpcServer.h
class RpcServer : noncopyable, public IMuduoUser
{
public:
    RpcServer(EventLoop* loop, const NetAddress& listenAddr);

    void registerService(::google::protobuf::Service*);
    void start();

private:
    virtual void onConnection(TcpConnectionPtr conn) override;
    virtual void onMessage(TcpConnectionPtr conn, Buffer* pBuf) override;
    virtual void onWriteComplate(TcpConnectionPtr conn) override;
    virtual void onClose(TcpConnectionPtr conn) override;

    TcpServer _server;
    std::map<std::string, ::google::protobuf::Service*> _services;
};
```

```cpp
RpcServer.cc
RpcServer::RpcServer(EventLoop* loop, const NetAddress& listenAddr)
  : _server(loop, listenAddr)
{
      //初始化TcpServer
}
void RpcServer::registerService(google::protobuf::Service* service)
{
    //将ServicesName 和 service 注册进 _services
    const google::protobuf::ServiceDescriptor* desc = service->GetDescriptor();
    _services[desc->full_name()] = service;
}
void RpcServer::onConnection(const TcpConnectionPtr conn)
{
    //当有客户端连接时创建一个RpcChannel 用于双端通信
    RpcChannelPtr channel(new RpcChannel(conn));
    channel->setServices(&_services);
    conn->setContext(channel);
}
void RpcServer::onMessage(const TcpConnectionPtr conn, Buffer* buf)
{
    //客户端有连接请求时找到对应的channel进行消息接收&处理
    RpcChannelPtr channel = boost::any_cast<RpcChannelPtr>(conn->getContext());
    channel->onMessage(conn, buf);
}
```



## 7.时序图

![](/img/时序图.png)

左半部分为client端的流程，右半部分为对应服务端的处理流程
