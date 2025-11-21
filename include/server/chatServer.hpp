#pragma once

#include <muduo/net/TcpServer.h>
#include <string>

using namespace muduo::net;
using muduo::Timestamp;

class ChatServer{
public:
    ChatServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg);
    void start();
private:

    void onConnection(const TcpConnectionPtr&);
    void onMessage(const TcpConnectionPtr&, Buffer*, Timestamp);

    TcpServer _server;
    EventLoop* _loop;
};


