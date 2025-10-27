#pragma once

#include <map>
#include <functional>
#include <muduo/net/TcpConnection.h>
#include <muduo/base/Logging.h>
#include <memory>
#include <mutex>

#include "json.hpp"
#include "userModel.hpp"
#include "offlineMessageModel.hpp"
#include "friendModel.hpp"
#include "group.hpp"
#include "groupModel.hpp"
#include "redis.hpp"

using json = nlohmann::json;

using namespace muduo;
using namespace muduo::net;

using MsgHandler = std::function<void(const TcpConnectionPtr&, json&, Timestamp)>;

// 注册各种业务服务 全局只需要一个实例
class ChatService{
public:
    static ChatService* getInstance();

    void exceptionExit();
    
    void login(const TcpConnectionPtr&, json&, Timestamp);
    
    void reg(const TcpConnectionPtr&, json&, Timestamp);

    void chat(const TcpConnectionPtr&, json&, Timestamp);

    void addFriend(const TcpConnectionPtr&, json&, Timestamp);

    void createGroup(const TcpConnectionPtr&, json&, Timestamp);

    void addGroup(const TcpConnectionPtr&, json&, Timestamp);

    void groupChat(const TcpConnectionPtr&, json&, Timestamp);

    void logout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    
    MsgHandler getHandler(int typeMsg);

    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr& conn);

    void handleRedisSubscribeMessage(int id, std::string msg);

private:
    ChatService();

    // 消息id对应的处理方法
    std::map<int, MsgHandler> _msgHandlerMap;

    // 存储在线用户的连接
    std::mutex _connMutex;
    std::map<int, TcpConnectionPtr>  _userConnMap;

    // 操作数据库
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;
    Redis _redis;
};