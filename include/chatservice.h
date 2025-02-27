#include <muduo/net/TcpServer.h>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "usermodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "offlinemessagemodel.hpp"

using namespace std;
using namespace muduo;
using namespace muduo::net;

#include "json.hpp"
using json = nlohmann::json;

// 处理消息的事件回调方法类型
using MsgHandler = function<void(const TcpConnectionPtr&, json&, Timestamp)>;

// 消息类型枚举
enum EnMsgType
{
    LOGIN_MSG = 1, // 登录消息 1
    LOGIN_MSG_ACK, // 登录响应消息 2
    LOGINOUT_MSG, // 注销消息 3
    REG_MSG, // 注册消息 4
    REG_MSG_ACK, // 注册响应消息 5
    ONE_CHAT_MSG, // 聊天消息 6
    ADD_FRIEND_MSG, // 添加好友消息 7

    CREATE_GROUP_MSG, // 创建群组 8
    ADD_GROUP_MSG, // 加入群组 9
    GROUP_CHAT_MSG, // 群聊天 10

    LOGINOK_MSG,  // 登录成功后发送的消息 11
    LOGINOK_MSG_ACK,   // 登录成功消息的响应 12
    NEW_USER_LOGIN_MSG,// 有新用户登录的消息 13
};

class ChatService
{
public:
    // 获取单例对象
    static ChatService* getInstance();
    // 处理注册业务
    void reg(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 处理登录业务
    void login(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 一对一聊天业务
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 创建群组业务
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 加入群组业务
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 群组聊天业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 客户端异常退出
    void clientCloseException(const TcpConnectionPtr& conn);
    // 处理注销业务
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);
private:
    ChatService();
    ChatService(const ChatService &) = delete;
    ChatService &operator=(const ChatService &) = delete;

    // 存储消息id和其对应的业务处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;
    // 存储在线用户的通信连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;
    // 定义互斥锁，保证_userConnMap的线程安全
    mutex _connMutex;

    //数据操作类对象
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;
};