#include "ProtobufCodecLite.h"

ProtobufCodecLite::ProtobufCodecLite(const RpcMeta::RpcMetaMessage* prototype,
                      std::string tagArg,
                      const ProtobufMessageCallback& messageCb,
                      const ErrorCallback& errorCb)
      : _prototype(prototype),
        _tag(tagArg),
        _messageCallback(messageCb),
        _errorCallback(errorCb),
        kMinMessageLen(tagArg.size() + kChecksumLen)
{
}

void ProtobufCodecLite::send(const TcpConnectionPtr& conn,
                             const RpcMeta::RpcMetaMessage& message)
{
    Buffer buf;
    fillEmptyBuffer(&buf, message);
    std::string msg(buf.retrieveAllAsString());
    conn->send(msg);
}

void ProtobufCodecLite::fillEmptyBuffer(Buffer* buf, const RpcMeta::RpcMetaMessage& message)
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
    std::cout << "onMessage size." << buf->readableBytes() << std::endl;
    while (buf->readableBytes() >= static_cast<uint32_t>(kMinMessageLen+kHeaderLen))
    {
        int32_t be32 = 0;
        ::memcpy(&be32, buf->peek(), sizeof(be32));
        const int32_t len = networkToHost32(be32);

        if (len > kMaxMessageLen || len < kMinMessageLen)
        {
            _errorCallback(conn, buf, kInvalidLength);
            break;
        }
        else if (buf->readableBytes() >= static_cast<size_t>(kHeaderLen+len))
        {
            RpcMetaMessagePtr message(_prototype->New());

            ErrorCode errorCode = parse(buf->peek()+kHeaderLen, len, message.get());  //解析
            if (errorCode == kNoError)
            {
                _messageCallback(conn, message);
                buf->retrieve(kHeaderLen+len);
            }
            else
            {
                _errorCallback(conn, buf, errorCode);
                break;
            }
        }
        else
        {
            break;
        }
    }
}

bool ProtobufCodecLite::parseFromBuffer(std::string buf, RpcMeta::RpcMetaMessage* message)
{
    return message->ParseFromArray(buf.data(), buf.size());
}

int ProtobufCodecLite::serializeToBuffer(const RpcMeta::RpcMetaMessage& message, Buffer* buf)
{
    // code copied from MessageLite::SerializeToArray() and MessageLite::SerializePartialToArray().
    GOOGLE_DCHECK(message.IsInitialized()) << InitializationErrorMessage("serialize", message);

    #if GOOGLE_PROTOBUF_VERSION > 3009002
        int byteSize = google::protobuf::internal::ToIntSize(message.ByteSizeLong());
    #else
        int byteSize = message.ByteSize();
    #endif
    
    //预购buffer容量
    buf->ensureWritableBytes(byteSize + kChecksumLen);

    uint8_t* start = reinterpret_cast<uint8_t*>(buf->beginWrite());
    uint8_t* end = message.SerializeWithCachedSizesToArray(start);
    if (end - start != byteSize)
    {
        #if GOOGLE_PROTOBUF_VERSION > 3009002
            ByteSizeConsistencyError(byteSize, google::protobuf::internal::ToIntSize(message.ByteSizeLong()), static_cast<int>(end - start));
        #else
            ByteSizeConsistencyError(byteSize, message.ByteSize(), static_cast<int>(end - start));
        #endif
    }
    buf->hasWritten(byteSize);
    return byteSize;
}

void ProtobufCodecLite::defaultErrorCallback(const TcpConnectionPtr& conn,
                                             Buffer* buf,
                                             ErrorCode errorCode)
{
    std::cout << "ProtobufCodecLite::defaultErrorCallback - " << errorCodeToString(errorCode) << std::endl;
    exit(-1);
}

int32_t ProtobufCodecLite::checksum(const void* buf, int len)
{
    return static_cast<int32_t>(::adler32(1, static_cast<const Bytef*>(buf), len));
}

bool ProtobufCodecLite::validateChecksum(const char* buf, int len)
{
    int32_t be32 = 0;
    ::memcpy(&be32, buf + len - kChecksumLen, sizeof(be32));
    int32_t expectedCheckSum = networkToHost32(be32);
    int32_t checkSum = checksum(buf, len - kChecksumLen);

    return checkSum == expectedCheckSum;
}

ProtobufCodecLite::ErrorCode ProtobufCodecLite::parse(const char* buf, int len, RpcMeta::RpcMetaMessage* message)
{
    ErrorCode error = kNoError;
    if (validateChecksum(buf, len))
    {
        if (memcmp(buf, _tag.data(), _tag.size()) == 0)
        {
            // parse from buffer
            const char* data = buf + _tag.size();
            int32_t dataLen = len - kChecksumLen - static_cast<int>(_tag.size());
            if (parseFromBuffer(std::string(data, dataLen), message))
            {
                error = kNoError;
            }
            else
            {
                error = kParseError;
            }
        }
        else
        {
            error = kUnknownMessageType;
        }
    }
    else
    {
        error = kCheckSumError;
    }

    return error;
}

namespace
{
    const string kNoErrorStr = "NoError";
    const string kInvalidLengthStr = "InvalidLength";
    const string kCheckSumErrorStr = "CheckSumError";
    const string kInvalidNameLenStr = "InvalidNameLen";
    const string kUnknownMessageTypeStr = "UnknownMessageType";
    const string kParseErrorStr = "ParseError";
    const string kUnknownErrorStr = "UnknownError";
}

const string& ProtobufCodecLite::errorCodeToString(ErrorCode errorCode)
{
    switch (errorCode)
    {
    case kNoError:
        return kNoErrorStr;
    case kInvalidLength:
        return kInvalidLengthStr;
    case kCheckSumError:
        return kCheckSumErrorStr;
    case kInvalidNameLen:
        return kInvalidNameLenStr;
    case kUnknownMessageType:
        return kUnknownMessageTypeStr;
    case kParseError:
        return kParseErrorStr;
    default:
        return kUnknownErrorStr;
    }
}
