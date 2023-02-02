#ifndef __CHATSERVICE_H__
#define __CHATSERVICE_H__

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>

using namespace std;
using namespace muduo;
using namespace muduo::net;

#include "redis.hpp"
#include "GroupModel.hpp"
#include "OfflineMsgModel.hpp"
#include "UserModel.hpp"
#include "FriendModel.hpp"
#include "json.hpp"
using json = nlohmann::json;    //起别名 using相当于typedef
// 表示处理消息的事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr& conn,json& js,Timestamp)>;

//聊天服务业务类
class ChatService{
public:
    // 获取单例ChatService对象的接口
    static ChatService* instance();
    // 登录业务
    void login(const TcpConnectionPtr& conn,json &js,Timestamp time);
    // 注册业务
    void reg(const TcpConnectionPtr& conn,json &js,Timestamp time);
    // 一对一聊天业务
    void oneChat(const TcpConnectionPtr& conn,json &js,Timestamp time);
    // 添加好友业务
    void addFriend(const TcpConnectionPtr& conn,json &js,Timestamp time);
    // 创建群组
    void createGroup(const TcpConnectionPtr& conn,json &js,Timestamp time);
    // 加入群组
    void addGroup(const TcpConnectionPtr& conn,json &js,Timestamp time);
    // 群组聊天
    void groupChat(const TcpConnectionPtr& conn,json &js,Timestamp time);
    // 注销
    void loginOut(const TcpConnectionPtr& conn,json &js,Timestamp time);
    // 客户端异常退出
    void clientCloseException(const TcpConnectionPtr& conn);
    // 服务器异常，业务重置
    void reset();
    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    // 从redis消息队列中获取订阅的消息
    void handlerRedisSubscribeMessage(int ,string);

private:
    ChatService();    //这是不让在外面随便实例化对象

    // 存储消息id及其对应的业务的处理方法
    unordered_map<int,MsgHandler> _msgHandlerMap;
    // 储存在线用户的通信连接
    unordered_map<int,TcpConnectionPtr> _userConnMap;
    // 互斥锁，保证_userConnMap的线程安全
    mutex _connMutex;

    // 数据操作类对象
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    // redis操作对象
    Redis _redis;
};


#endif