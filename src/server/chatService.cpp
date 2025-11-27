#include "chatService.hpp"
#include "public.hpp"

#include <iostream>
#include <vector>

ChatService* ChatService::getInstance(){
    static ChatService service;
    return &service;

}

void ChatService::exceptionExit(){
    _userModel.resetAllState();
}

ChatService::ChatService(){

    using namespace std::placeholders;
    _msgHandlerMap.insert({MSG_LOGIN, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({MSG_LOGOUT, std::bind(&ChatService::logout, this, _1, _2, _3)});
    _msgHandlerMap.insert({MSG_REG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({MSG_CHAT, std::bind(&ChatService::chat, this, _1, _2, _3)});
    _msgHandlerMap.insert({MSG_ADD_FRIEND, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

     // 群组业务管理相关事件处理回调注册
    _msgHandlerMap.insert({MSG_CREATE_GROUP, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({MSG_ADD_GROUP, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({MSG_GROUP_CHAT, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({MSG_PING, std::bind(&ChatService::ping, this, _1, _2, _3)});

    if(_redis.connect()){
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
    else{
        LOG_FATAL << "connecting redis fail";
        
    }
}

void ChatService::ping(const TcpConnectionPtr& conn, json& js, Timestamp){
    // 收到客户端心跳，更新活动时间并回复 PONG
    updateConnectionActivity(conn);
    json resp;
    resp["msgid"] = MSG_PONG;
    conn->send(resp.dump() + "\r\n");
}

void ChatService::login(const TcpConnectionPtr& conn, json& js, Timestamp){
    LOG_INFO << "do login service";

    int id = js["id"];
    std::string passwd = js["password"];
    
    std::optional<User> user = _userModel.query(id);
    if(user.has_value() && user.value().getPassword() == passwd){
        if(user.value().getState() == "online"){
            json response;
            response["msgid"] = MSG_ACK_LOGIN;
            response["errno"] = 2;
            response["errmsg"] = "该帐号已经登录";
            conn->send(response.dump() + "\r\n");
        }
        else{
            // 登录成功
            {
                std::lock_guard<std::mutex> guard(_connMutex);
                _userConnMap.insert({id, conn});
                _connLastActiveMap[conn] = time(nullptr);
            }

            // 向redis订阅
            _redis.subscribe(id);

            _userModel.updateState(id, "online");

            json response;
            response["msgid"] = MSG_ACK_LOGIN;
            response["errno"] = 0;
            response["id"] = id;
            response["name"] = user.value().getName();

            // 查询离线消息
            std::vector<std::string> msgs = _offlineMsgModel.query(id);
            response["offlinemsg"] = msgs;
            _offlineMsgModel.clear(id);

            // 查询好友信息
            std::vector<User> friends = _friendModel.query(id);
            std::vector<std::string> temp;
            for(auto& user : friends){
                json js;
                js["id"] = user.getId();
                js["name"] = user.getName();
                js["state"] = user.getState(); 
                temp.push_back(js.dump());
            }
            response["friends"] = temp;

            // 查询用户的群组信息
            std::vector<Group> groupuserVec = _groupModel.queryGroups(id);
            if (!groupuserVec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                std::vector<string> groupV;
                for (Group &group : groupuserVec)
                {
                    json grpjson;
                    grpjson["id"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();
                    std::vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());
                }

                response["groups"] = groupV;
            }

            conn->send(response.dump() + "\r\n");
        }
    }
    else {
        json response;
        response["msgid"] = MSG_ACK_LOGIN;
        response["errno"] = 1;
        response["errmsg"] = "用户名或密码错误";
        conn->send(response.dump() + "\r\n");
    }
}

void ChatService::reg(const TcpConnectionPtr& conn, json& js, Timestamp){
    LOG_INFO << "do reg service";

    std::string name = js["name"];
    std::string password = js["password"];

    User user;
    user.setName(name);
    user.setPassword(password);
    if(_userModel.insert(user)){
        json response;
        response["msgid"] = MSG_ACK_REG;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump() + "\r\n");
    }
    else{
        json response;
        response["msgid"] = MSG_ACK_REG;
        response["errno"] = 1;
        conn->send(response.dump() + "\r\n");
    }
}

MsgHandler ChatService::getHandler(int typeMsg){
    if(!_msgHandlerMap.count(typeMsg)){
        return [typeMsg](auto a, auto b, auto c){
            LOG_ERROR << typeMsg << " can not find handler";
        };
    }
    return _msgHandlerMap[typeMsg];
}

void ChatService::clientCloseException(const TcpConnectionPtr& conn){
    int id = -1;
    {
        std::lock_guard<std::mutex> guard(_connMutex);
        if(auto it = std::find_if(_userConnMap.begin(), _userConnMap.end(), [&conn](const auto& a){return a.second == conn;}); it != _userConnMap.end()){
            id = it->first;
            _userConnMap.erase(it);
            _connLastActiveMap.erase(conn);
        }
    }
    // 向redis取消订阅
    _redis.unsubscribe(id);
    _userModel.updateState(id, "offline");
}

void ChatService::chat(const TcpConnectionPtr& conn, json& js, Timestamp){
    int to = js["toid"];
    {
        // 在线直接转发消息
        std::lock_guard<std::mutex> guard(_connMutex);
        if(auto it = _userConnMap.find(to); it != _userConnMap.end()){
            it->second->send(js.dump() + "\r\n");
            return;
        }
    }

    // 对方链接可能在其他服务器，检查是否登录
    std::optional<User> user = _userModel.query(to);
    if(user.has_value() && user.value().getState() == "online"){
        _redis.publish(to, js.dump());
        return;
    }

    // 不在线存储离线消息
    _offlineMsgModel.insert(to, js.dump());
}

void ChatService::addFriend(const TcpConnectionPtr& conn, json& js, Timestamp){
    int userid = js["id"];
    int friendid = js["friendid"];

    _friendModel.insert(userid, friendid);
}

void ChatService::createGroup(const TcpConnectionPtr& conn, json& js, Timestamp){
    int id = js["id"];
    std::string name = js["groupname"];
    std::string desc = js["groupdesc"];

    Group group(-1, name, desc);
    if(_groupModel.createGroup(group)){
        _groupModel.addGroup(id, group.getId(), "creator");
    }
}

void ChatService::addGroup(const TcpConnectionPtr& conn, json& js, Timestamp){
    int id = js["id"];
    int groupid = js["groupid"];
    _groupModel.addGroup(id, groupid, "normal");
}

void ChatService::groupChat(const TcpConnectionPtr& conn, json& js, Timestamp){
    int userid = js["id"];
    int groupid = js["groupid"];
    std::vector<int> idVec = _groupModel.queryGroupUsers(userid, groupid);
    
    // 向所有成员转发
    std::lock_guard<std::mutex> guard(_connMutex);
    for(int id : idVec){
        if(auto it = _userConnMap.find(id); it != _userConnMap.end()){
            it->second->send(js.dump() + "\r\n");
        } 
        else{
            // 检查是否在线
            std::optional<User> user = _userModel.query(id);
            if(user.has_value() && user.value().getState() == "online"){
                _redis.publish(id, js.dump());
            }
            else{
                _offlineMsgModel.insert(id, js.dump()); 
            }
        }
    }
}

// 处理登出业务
void ChatService::logout(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"].get<int>();
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _connLastActiveMap.erase(it->second);
            _userConnMap.erase(it);
        }
    }

    // 向redis取消订阅
    _redis.unsubscribe(userid);

    // 更新用户的状态信息
    _userModel.updateState(userid, "offline");
}

void ChatService::handleRedisSubscribeMessage(int id, std::string msg){
    json js = json::parse(msg.c_str());

    std::lock_guard<std::mutex> guard(_connMutex);
    if(auto it = _userConnMap.find(id); it != _userConnMap.end()){
        it->second->send(msg + "\r\n");
    }
    else{
        // 有可能突然下线 保存消息
        _offlineMsgModel.insert(id, msg);
    }
}

void ChatService::updateConnectionActivity(const TcpConnectionPtr& conn){
    std::lock_guard<std::mutex> guard(_connMutex);
    auto it = _connLastActiveMap.find(conn);
    if(it != _connLastActiveMap.end()){
        it->second = time(nullptr);
    }
}

void ChatService::checkIdleConnections(int idleSeconds){
    std::vector<TcpConnectionPtr> toClose;
    time_t now = time(nullptr);
    {
        std::lock_guard<std::mutex> guard(_connMutex);
        for(auto &p : _connLastActiveMap){
            if(now - p.second > idleSeconds){
                toClose.push_back(p.first);
            }
        }
    }

    for(auto &conn : toClose){
        LOG_INFO << "closing idle connection";
        // 触发清理
        clientCloseException(conn);
        conn->shutdown();
    }
}
