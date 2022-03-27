#ifndef RPCCHANNEL_H
#define RPCCHANNEL_H

#include "ProtobufCodecLite.h"
#include "ICommonCallback.h"
#include "Mutex.h"
#include "RpcMeta.pb.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <map>

// Service and RpcChannel classes are incorporated from
// google/protobuf/service.h

// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

//一个Tcp客户端连接对应一个RpcChannel
class RpcChannel : public google::protobuf::RpcChannel
{
public:
    RpcChannel();
    explicit RpcChannel(const TcpConnectionPtr& conn);
    ~RpcChannel() override;

    void setConnection(const TcpConnectionPtr& conn)
    {
        _conn = conn;
    }

    void setServices(const std::map<std::string, google::protobuf::Service*>* services)
    {
        _services = services;
    }

    void CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller,
                    const google::protobuf::Message* request,
                    google::protobuf::Message* response,
                    google::protobuf::Closure* done) override;

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf);

private:
    // 功能1：服务端处理REQUEST请求 功能2：客户端处理RESPONSE响应
    void onRpcMessage(const TcpConnectionPtr& conn, const RpcMetaMessagePtr& messagePtr);
    // 服务端接收到 REQUEST 请求时会调用 service 的 callmethod 方法，进而执行客户override 的rpc方法
    // 当客户的rpc方法执行完后会调用一个执行完成的回调，调用的是这个接口
    // 功能为销毁response内存，构造rpcmeta中相应内容，利用网络库发送response给客户端
    void doneCallback(google::protobuf::Message* response, int64_t id);

    int64_t incrementAndGet() 
    { 
        return __sync_fetch_and_add(&_id, 1) + 1; 
    }

    struct OutstandingCall
    {
        google::protobuf::Message* response;  //请求对应的response内存
        google::protobuf::Closure* done;      //接收到服务端返回response后需要执行的用户回调函数
    };

    ProtobufCodecLite _codec;
    TcpConnectionPtr _conn;
    int64_t _id;
    MutexLock _mutex;
    //客户端使用
    //向服务器请求服务时带上此次请求的 sequenceid
    //同时将对应的 OutstandingCall 结构存储到该结构中
    //当服务端返回数据时根据 RpcMeta 中的 sequenceid 找到该结构然后做相应操作
    std::map<int64_t, OutstandingCall> _outstandings; 
    //服务端使用
    //服务端每定义一个 Service 都会注册到 RpcServer 中
    //当有客户端连接时 RpcServer 会为该连接创建一个 RpcChannel 然后将所有的Services给下来
    //客户端发起请求时服务端通过meta中的serviceName信息找到对应的内容，然后调用对应的CallMethod方法
    const std::map<std::string, google::protobuf::Service*>* _services;
};

typedef std::shared_ptr<RpcChannel> RpcChannelPtr;

#endif  // RPCCHANNEL_H
