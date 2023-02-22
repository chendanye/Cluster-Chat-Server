#include "chatserver.hpp"
#include "chatservice.hpp"

#include <iostream>
#include <string>
#include <signal.h>

using namespace std;

// 再包装一层是因为signal()
void resetHandler(int){
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc,char** argv)
{
    if(argc < 3){
        cerr << "command invalid! example: ./ChatServer ip port" << endl;
        exit(-1);
    }
    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT,resetHandler);

    EventLoop loop;
    InetAddress addr(ip,port);
    ChatServer server(&loop,addr,"ChatServer");

    //
    server.start();
    loop.loop();

    return 0;
}