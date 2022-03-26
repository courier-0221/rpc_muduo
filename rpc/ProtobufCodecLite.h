#ifndef PROTOBUFCODECLITE_H
#define PROTOBUFCODECLITE_H

#include "Timestamp.h"
#include "Common.h"
#include "TcpConnection.h"
#include "google-inl.h"
#include "RpcMeta.pb.h"

#include <string>
#include <memory>
#include <type_traits>
#include <google/protobuf/message.h>
#include <zlib.h>

typedef std::shared_ptr<RpcMeta::RpcMetaMessage> RpcMetaMessagePtr;

// wire format
//
// Field     Length  Content
//
// size      4-byte  M+N+4
// tag       M-byte  could be "RPC0", etc.
// payload   N-byte
// checksum  4-byte  adler32 of tag+payload
class ProtobufCodecLite : noncopyable
{
public:
    const static int kHeaderLen = sizeof(int32_t);
    const static int kChecksumLen = sizeof(int32_t);
    const static int kMaxMessageLen = 64*1024*1024;

    enum ErrorCode
    {
        kNoError = 0,
        kInvalidLength,
        kCheckSumError,
        kInvalidNameLen,
        kUnknownMessageType,
        kParseError,
    };

    typedef std::function<void (const TcpConnectionPtr&, const RpcMetaMessagePtr&)> ProtobufMessageCallback;
    typedef std::function<void (const TcpConnectionPtr&, Buffer*, ErrorCode)> ErrorCallback;

    ProtobufCodecLite(const RpcMeta::RpcMetaMessage* prototype,
                      std::string tagArg,
                      const ProtobufMessageCallback& messageCb,
                      const ErrorCallback& errorCb = defaultErrorCallback);

    virtual ~ProtobufCodecLite() = default;

    const string& tag() const { return _tag; }

    void send(const TcpConnectionPtr& conn, const RpcMeta::RpcMetaMessage& message);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf);

    //序列化与反序列化
    bool parseFromBuffer(std::string buf, RpcMeta::RpcMetaMessage* message);
    int serializeToBuffer(const RpcMeta::RpcMetaMessage& message, Buffer* buf);

    ErrorCode parse(const char* buf, int len, RpcMeta::RpcMetaMessage* message);
    void fillEmptyBuffer(Buffer* buf, const RpcMeta::RpcMetaMessage& message);

    //网络传输过来checksum内容的和本地算出来的是否为一致的
    static int32_t checksum(const void* buf, int len);
    static bool validateChecksum(const char* buf, int len);
    static void defaultErrorCallback(const TcpConnectionPtr&, Buffer*, ErrorCode);
    static const string& errorCodeToString(ErrorCode errorCode);

private:
    const RpcMeta::RpcMetaMessage* _prototype;
    const string _tag;
    ProtobufMessageCallback _messageCallback;
    ErrorCallback _errorCallback;
    const int kMinMessageLen;
};

#endif  // PROTOBUFCODECLITE_H
