#include "chatServer.hpp"
#include "chatService.hpp"

#include <iostream>
#include <signal.h>

void exceptionExitHandler(int){
    ChatService::getInstance()->exceptionExit();
    exit(0);
}

int main(int argc, char** argv){
    if(argc < 3){
        std::cerr << "command invalid! exanple: ./ChatServer 127.0.0.1 6000" << std::endl;
    }

    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, exceptionExitHandler);

    EventLoop loop;
    InetAddress addr(port, ip);
    ChatServer server(&loop, addr, "chatServer");
    server.start();
    loop.loop();
}