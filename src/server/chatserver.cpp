#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"

#include <iostream>
#include <functional>
#include <string>

using namespace std;
using namespace placeholders;

using json = nlohmann::json;

// 初始化
ChatServer::ChatServer(EventLoop* loop,const InetAddress& listenAddr,const string& namearg)
                        : _server(loop,listenAddr,namearg),_loop(loop)
{
    //注册连接回调函数
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,_1));
    //注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));
    //设置线程数量
    _server.setThreadNum(4);
    return;
}

// 启动服务
void ChatServer::start(){
    _server.start();
    return;
}



// 上报连接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr& conn)
{
    // 客户端异常 断开链接
    if(!conn->connected()) 
    {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }

    // 其他

    return;
}

// 上报读写相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr& conn,Buffer* buf,Timestamp time)
{
    string buffer = buf->retrieveAllAsString();

    // 数据的反序列化
    json js = json::parse(buffer);
    // 获取消息相应的回调函数
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    // 执行相应的回调
    msgHandler(conn,js,time);
    return;
}

