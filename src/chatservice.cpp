#include "chatservice.h"
#include "user.hpp"
#include <iostream>
#include <muduo/base/Logging.h>

ChatService *ChatService::getInstance() {
    // 静态局部变量的初始化，在汇编指令上已经自动添加线程互斥指令了
    static ChatService service;
    return &service;
}

// 注册业务
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    string name = js["name"];
    string pwd = js["password"];

    cout << "username: " << name << "pwd: " << pwd << endl;
    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);
    if (state)
    {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else 
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

// 客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn) {
{
        lock_guard<mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); it++)
        {
            if (it->second == conn)
            {
                // 从map表中删除用户的连接信息
                _userConnMap.erase(it);
                break;
            }
        }
    }
}

// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid) {
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // 返回一个默认的处理器，空操作
        return [=](const TcpConnectionPtr& conn, json& js, Timestamp time)
        {
            LOG_ERROR << "msgid:" << msgid << " can not find handler!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}

ChatService::ChatService() {

}
