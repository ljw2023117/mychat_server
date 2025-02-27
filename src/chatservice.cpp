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
    cout << __FUNCTION__ << endl;
    string name = js["name"];
    string pwd = js["password"];

    cout << "username: " << name << "pwd: " << pwd << endl;
    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);
    cout << "insert state: " << state << endl;
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

// 登录业务
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    cout << __FUNCTION__ << endl;
    int id = js["id"].get<int>();
    string pwd = js["password"];

    User user = _userModel.query(id);
    if (user.getId() == id && user.getPwd() == pwd)
    {
        if (user.getState() == "online")
        {
            // 该用户已经登录，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "this account is using, input another!";
            conn->send(response.dump());
        }
        else
        {
            // 登录成功 发送消息
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            conn->send(response.dump());
            
            // 登录成功，记录用户连接信息
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }
            // 登录后给所有当前在线的用户发消息
            json response;
            response["msgid"] = NEW_USER_LOGIN_MSG;
            response["id"] = user.getId();
            response["username"] = user.getName();
            for (auto& t: _userConnMap)
            {
                t.second->send(response.dump());
            }
            
            // 登录后给新登录的用户发送当前在线用户的信息
            for (auto& t: _userConnMap)
            {
                User user = _userModel.query(t.first);
                json response;
                response["msgid"] = LOGINOK_MSG_ACK;
                response["id"] = user.getId();
                response["username"] = user.getName();
                conn->send(response.dump());
            }
            
            // 登录成功，更新用户状态信息  state offline=>online
            user.setState("online");
            _userModel.updateState(user);



        }
    }
    else
    {
        // 登录失败 
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "id or password is invalid!";
        conn->send(response.dump());
    }
}

void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time) {

    int toid = js["toid"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // toid在线，转发消息 服务器主动推送消息给toid用户
            it->second->send(js.dump());
            return;
        }
    }

    // // 查询toid是否在线
    // User user = _userModel.query(toid);
    // if (user.getState() == "online")
    // {
    //     _redis.publish(toid, js.dump());
    //     return;
    // }

    // _offlineMsgModel.insert(toid, js.dump());

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

void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    _userModel.updateState(user);
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
    // 用户基本业务管理相关事件处理回调注册
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    // _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

    // // 群组业务管理相关事件处理回调函数
    // _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    // _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    // _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

}
