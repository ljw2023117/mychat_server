#include <muduo/net/TcpServer.h>
#include <unordered_map>
#include <functional>
#include <mutex>

using namespace std;
using namespace muduo;
using namespace muduo::net;

#include "json.hpp"
using json = nlohmann::json;

// 处理消息的事件回调方法类型
using MsgHandler = function<void(const TcpConnectionPtr&, json&, Timestamp)>;

class ChatService
{
public:
private:
  ChatService(const ChatService &) = delete;
  ChatService &operator=(const ChatService &) = delete;
  ChatService();
  // 存储消息id和其对应的业务处理方法
  unordered_map<int, MsgHandler> _msgHandlerMap;
  // 存储在线用户的通信连接
  unordered_map<int, TcpConnectionPtr> _userConnMap;
  // 定义互斥锁，保证_userConnMap的线程安全
  mutex _connMutex;
};