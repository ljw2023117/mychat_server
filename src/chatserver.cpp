#include "chatserver.h"
#include "json.hpp"
#include <iostream>
#include "chatservice.h"

using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg)
    :_server(loop, listenAddr, nameArg)
{
    // 注册链接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
    // 注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    // 设置线程数量
    _server.setThreadNum(4);
}

void ChatServer::start() {
    _server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr& conn)
{
    // 客户端断开链接
    if (!conn->connected()){
        ChatService::getInstance()->clientCloseException(conn);
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    // 数据的反序列化
    json js = json::parse(buf);
    // 通过js["msgid"] 获取业务handler
    auto msgHandler = ChatService::getInstance()->getHandler(js["msgid"].get<int>());
    cout << __FUNCTION__ << js["msgid"].get<int>() << endl;
    msgHandler(conn, js, time);
}
