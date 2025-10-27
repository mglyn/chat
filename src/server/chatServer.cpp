#include "chatServer.hpp"
#include "chatService.hpp"
#include "json.hpp"

#include <functional>
#include <string>
#include <iostream>

using json = nlohmann::json;

ChatServer::ChatServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg):
    _server(loop, listenAddr, nameArg)
{
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
    _server.setMessageCallback(
        std::bind(&ChatServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    //_server.setThreadNum(1);
}

void ChatServer::start(){
    _server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr& conn){
    // 客户端断开连接
    if(!conn->connected()){
        ChatService::getInstance()->clientCloseException(conn);
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time){

    std::string buf = buffer->retrieveAllAsString();
    // 反序列化
    json js = json::parse(buf);
    // 网络模块业务模块解耦
    // 获得各种业务的handler并调用
    ChatService::getInstance()->getHandler(js["msgid"].get<int>())(conn, js, time);
}