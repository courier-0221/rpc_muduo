#include "RpcMeta.pb.h"
#include "ProtobufCodecLite.h"
#include "Buffer.h"
#include <stdio.h>

RpcMetaMessagePtr g_msgptr;
void rpcMessageCallback(const TcpConnectionPtr&, const RpcMetaMessagePtr& msg)
{
    g_msgptr = msg;
    std::cout << "type." << g_msgptr->type() << " id." << g_msgptr->id() << std::endl;
}

void print(Buffer& buf)
{
    printf("encoded to %zd bytes\n", buf.readableBytes());
    for (size_t i = 0; i < buf.readableBytes(); ++i)
    {
        unsigned char ch = static_cast<unsigned char>(buf.peek()[i]);
        printf("%2zd:  0x%02x  %c\n", i, ch, isgraph(ch) ? ch : ' ');
    }
}

int main()
{
    RpcMeta::RpcMetaMessage message;
    message.set_type(RpcMeta::REQUEST);
    message.set_id(2);
    Buffer buf;
    {
        ProtobufCodecLite codec(&RpcMeta::RpcMetaMessage::default_instance(), "RPC0", rpcMessageCallback);
        codec.fillEmptyBuffer(&buf, message);
        print(buf);
        codec.onMessage(TcpConnectionPtr(), &buf);
        assert(g_msgptr);
        assert(g_msgptr->DebugString() == message.DebugString());
        g_msgptr.reset();
    }

    printf("\n");

    {
        Buffer buf;
        ProtobufCodecLite codec(&RpcMeta::RpcMetaMessage::default_instance(), "XYZ", rpcMessageCallback);
        codec.fillEmptyBuffer(&buf, message);
        print(buf);
        codec.onMessage(TcpConnectionPtr(), &buf);
        assert(g_msgptr);
        assert(g_msgptr->DebugString() == message.DebugString());
    }

    google::protobuf::ShutdownProtobufLibrary();
}
