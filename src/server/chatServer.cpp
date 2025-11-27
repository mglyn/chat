#include "chatServer.hpp"
#include "chatService.hpp"
#include "json.hpp"

#include <muduo/net/EventLoop.h>

#include <functional>
#include <string>
#include <iostream>
#include <thread>

using json = nlohmann::json;

ChatServer::ChatServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg):
    _server(loop, listenAddr, nameArg), _loop(loop)
{
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
    _server.setMessageCallback(
        std::bind(&ChatServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    
    _server.setThreadNum(std::thread::hardware_concurrency());

    // 定时检查心跳，清理超过 60s 未活动的连接
    constexpr int IDLE_SECONDS = 60;
    _loop->runEvery(10.0, [=]() {
        ChatService::getInstance()->checkIdleConnections(IDLE_SECONDS);
    });
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
    while (const char* crlf = buffer->findCRLF())
    {
        std::string request(buffer->peek(), crlf - buffer->peek());
        buffer->retrieveUntil(crlf + 2);

        // 反序列化
        json js = json::parse(request);
        // 更新连接活动时间（心跳）
        ChatService::getInstance()->updateConnectionActivity(conn);
        // 网络模块业务模块解耦
        // 获得各种业务的handler并调用
        ChatService::getInstance()->getHandler(js["msgid"].get<int>())(conn, js, time);
    }
}