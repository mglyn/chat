#include "chatServer.hpp"
#include "chatService.hpp"


#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>
#include <iostream>
#include <signal.h>

void exceptionExitHandler(int){
    ChatService::getInstance()->exceptionExit();
    exit(0);
}

int main(int argc, char** argv){
    if(argc < 3){
        LOG_FATAL << "command invalid! exanple: ./ChatServer 127.0.0.1 6000";
    }

    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, exceptionExitHandler);

    EventLoop loop;
    InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "chatServer");
    server.start();
    loop.loop();
}