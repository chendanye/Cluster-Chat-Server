#ifndef __CHATSERVER_H__
#define __CHATSERVER_H__

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

// 服务器的主类
class ChatServer{
public:
    // 初始化
    ChatServer(EventLoop* loop,const InetAddress& listenAddr,const string& namearg);
    // 启动服务
    void start();
private:
    // 上报连接相关信息的回调函数
    void onConnection(const TcpConnectionPtr& conn);
    // 上报读写相关信息的回调函数
    void onMessage(const TcpConnectionPtr& conn,Buffer* buf,Timestamp time);

    // 组合的muduo库，实现服务器功能的类对象
    TcpServer _server;
    // 指向事件循环对象的指针
    EventLoop* _loop;
};


#endif
