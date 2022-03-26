#include "TcpClient.h"
#include "CurrentThread.h"

class TestClient : public IMuduoUser, IChannelCallback
{
 public:
	TestClient(EventLoop* loop, const NetAddress& serverAddr)
		: _loop(loop)
		, _client(_loop, serverAddr)
		, _stdinChannel(loop, 0)
	{
		_client.setCallback(this);

		//标准输入缓冲区中有数据的时候，回调TestClient::handleRead
		_stdinChannel.setCallback(this);
		_stdinChannel.enableReading();		// 关注可读事件
	}

	void connect()
	{
		_client.connect();
	}

	virtual void onConnection(TcpConnectionPtr conn)
	{
		cout << "TestClient::onConnection" << endl;
	}

    virtual void onMessage(TcpConnectionPtr conn, Buffer* pBuf)
	{
		string msg(pBuf->retrieveAllAsString());
		cout << "TestClient::onMessage" << "recv tid = " << CurrentThread::tid() 
				<< " recvSize: " << msg.size() << " bytes " << " recvData: " << msg.c_str() << endl;
	}

    virtual void onWriteComplate(TcpConnectionPtr conn)
	{
		cout << "TestClient::onWriteComplate" << endl;
	}

    virtual void onClose(TcpConnectionPtr conn) {cout << "TestClient::onClose" << endl;}

	// 标准输入缓冲区中有数据的时候，回调该函数
	virtual void handleRead()
	{
		char buf[1024] = {0};
		fgets(buf, 1024, stdin);
		buf[strlen(buf)-1] = '\0';		// 去除\n
		TcpConnectionPtr conn = _client.connection();
		if (conn.get()) conn->send(buf);
	}

	virtual void handleWrite() {}
    virtual void handleClose() {}

 private:
	EventLoop* _loop;
	TcpClient _client;
	Channel _stdinChannel;		// 标准输入Channel
};

int main(int argc, char* argv[])
{
	cout << "main pid = " << ::getpid() << " tid = " << CurrentThread::tid() << endl;
	
	EventLoop loop;
	NetAddress serverAddr("127.0.0.1", 11111);
	TestClient client(&loop, serverAddr);
	client.connect();
	loop.loop();

	return 0;
}