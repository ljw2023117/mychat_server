#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <memory>
#include "packet.h"
using namespace muduo;
using namespace muduo::net;
using namespace std;

class FileServer
{
public:
    FileServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg);
    void recvfile (CPacket&);
    void sendfile (const TcpConnectionPtr&, CPacket&);
    void start();

private:
    // 链接的回调函数
    void onConnection(const TcpConnectionPtr &);
    // 读写事件的回调函数
    void onMessage(const TcpConnectionPtr &, Buffer *, Timestamp);
    TcpServer _server;
    shared_ptr<EventLoop> eventloop;
};