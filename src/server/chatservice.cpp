#include "chatservice.hpp"
#include "public.hpp"
#include "user.hpp"
#include <muduo/base/Logging.h>
#include <vector>

using namespace std;
using namespace muduo;

// ChatService的构造函数：注册消息及其对应的回调句柄
ChatService::ChatService(){
    // 用户基本业务管理相关事件处理回调注册
    _msgHandlerMap.insert({LOGIN_MSG,     std::bind(&ChatService::login,    this,_1,_2,_3)});
    _msgHandlerMap.insert({LOGINOUT_MSG,  std::bind(&ChatService::loginOut, this,_1,_2,_3)});
    _msgHandlerMap.insert({REG_MSG,       std::bind(&ChatService::reg,      this,_1,_2,_3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG,  std::bind(&ChatService::oneChat,  this,_1,_2,_3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG,std::bind(&ChatService::addFriend,this,_1,_2,_3)});

    // 群组业务管理及其相关事件处理回调注册
    _msgHandlerMap.insert({CREATE_GROUP_MSG,std::bind(&ChatService::createGroup,this,_1,_2,_3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG,   std::bind(&ChatService::addGroup,   this,_1,_2,_3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG,  std::bind(&ChatService::groupChat,  this,_1,_2,_3)});

    // 连接redis服务器
    if(_redis.connect()){
        // 设置上报消息回调函数
        _redis.init_notify_handler(std::bind(&ChatService::handlerRedisSubscribeMessage,this,_1,_2));
    }

    return ;
}

// 获取单例ChatService对象的接口
ChatService* ChatService::instance()
{
    static ChatService cs;
    return &cs;
}


// 登录业务
void ChatService::login(const TcpConnectionPtr& conn,json &js,Timestamp time){
    //从json中获取id pwd 
    int id = js["id"].get<int>();   
    string pwd = js["passward"];

    // 根据id查询出用户信息
    User user = _userModel.queryUser(id); 
    //判断用户的id和密码是否一致
    if(user.getId() == id && user.getPwd() == pwd)
    {
        if(user.getState() == "online")  //该用户已登录，不应该重复登录
        {
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = ERROR_LOGIN;   
            response["errmsg"] = "this account is using,input another!";
            conn->send(response.dump());
        }
        else{
            // 登录信息验证成功，记录用户连接信息
            lock_guard<mutex> lock(_connMutex);  
            _userConnMap.insert({id,conn});

            // 用户登录成功后，向redis订阅channel(id)
            _redis.subscribe(id);
            user.setState("online");
            _userModel.updateState(user);  //用户状态改为"online"

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = NO_ERROR;   
            response["id"] = user.getId();
            response["name"] = user.getName();

            // 查询用户的离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if(!vec.empty()){
                response["offlinemsg"] = vec;
                // 读取用户的离线消息后把该用户所有的离线消息删掉
                _offlineMsgModel.remove(id);
            }

            // 查询用户的好友信息并返回
            vector<User> userVec = _friendModel.queryFriends(id);
            if(!userVec.empty()){
                vector<string> vec2;
                for(User& user : userVec){
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            // 查询用户的群组信息
            vector<Group> groupUserVec = _groupModel.QueryGroups(id);
            if(!groupUserVec.empty()){
                vector<string> vec3;
                for(Group& group : groupUserVec){
                    json js;
                    js["id"] = group.getId();
                    js["groupname"] = group.getGroupName();
                    js["groupdesc"] = group.getGroupDesc();
                    vector<string> userVec;
                    for(GroupUser& guser : group.getGroupUser()){
                        json jsuser;
                        jsuser["id"] = guser.getId();
                        jsuser["name"] = guser.getName();
                        jsuser["state"] = guser.getState();
                        jsuser["role"] = guser.getRole();
                        userVec.push_back(jsuser.dump());
                    }
                    js["users"] = userVec;
                    vec3.push_back(js.dump());
                }
                response["groups"] = vec3;
            }
            conn->send(response.dump());
        }
    }
    else{  //登录失败
        // 该用户不存在，或者是用户存在但是密码错误
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = ERROR_REGISTER; 
        response["errmsg"] = "id or passward is invalid!";
        conn->send(response.dump());
    }
    return;
}

// 注册业务
void ChatService::reg(const TcpConnectionPtr& conn,json &js,Timestamp time){
    string name = js["name"];
    string pwd = js["passward"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    //将注册信息添加到数据库中
    bool state = _userModel.insert(user);
    if(state)    //注册成功
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = NO_ERROR;   
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else{
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = ERROR_REGISTER;   
    }
    return;
}

// 一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr& conn,json &js,Timestamp time){
    int toid = js["toid"].get<int>();

    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(toid);
    if(it != _userConnMap.end()){
        // 对方在线 服务器直接转发消息给toid用户
        it->second->send(js.dump());
        return ;
    }

    // 查询toid是否在线
    User user = _userModel.queryUser(toid);
    if(user.getState() == "online"){
        _redis.publish(toid,js.dump());
        return ;
    }

    // toid不在线 存储为离线消息
    _offlineMsgModel.insert(toid,js.dump());
    return ;
} 

// 添加好友业务
void ChatService::addFriend(const TcpConnectionPtr& conn,json &js,Timestamp time){
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    // 存储好友信息
    _friendModel.insert(userid,friendid);
    return;
}

// 创建群组
void ChatService::createGroup(const TcpConnectionPtr& conn,json &js,Timestamp time){
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];
    //
    Group group;
    group.setGroupName(name);
    group.setGroupDesc(desc);
    if(_groupModel.CreateGroup(group)){
        _groupModel.AddGroup(userid,group.getId(),"creator");
    }
    return ;
}

// 加入群组
void ChatService::addGroup(const TcpConnectionPtr& conn,json &js,Timestamp time){
    int id = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.AddGroup(id,groupid,"normal");
    return ;
}

// 群组聊天
void ChatService::groupChat(const TcpConnectionPtr& conn,json &js,Timestamp time){
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid,groupid);
    lock_guard<mutex> lock(_connMutex);
    for(int id:useridVec){
        auto it = _userConnMap.find(id);
        if(it != _userConnMap.end()){
            it->second->send(js.dump());
        }
        else{
            User user = _userModel.queryUser(id);
            if(user.getState() == "online"){
                _redis.publish(id,js.dump());
            }
            else{
                _offlineMsgModel.insert(id,js.dump());
            }
        }
    }
    return;
}

// 注销
void ChatService::loginOut(const TcpConnectionPtr& conn,json &js,Timestamp time){
    int userid = js["id"].get<int>();

    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if(it != _userConnMap.end()){
        // 在map中删除该连接
        _userConnMap.erase(it);       
    }

    // 取消订阅该通道
    _redis.unsubscribe(userid);
    // 更新用户的状态为offline
    User user(userid,"","","offline");
    _userModel.updateState(user);
}

// 客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr& conn){
    User user;
    lock_guard<mutex> lock(_connMutex);
    for(auto it = _userConnMap.begin();it != _userConnMap.end();++it)
    {
        // 找到异常退出的客户端连接
        if(it->second == conn)
        {
            user.setId(it->first);  //记录客户端的id
            _userConnMap.erase(it); //从map中删除连接信息
            break;
        }
    }
    // 取消订阅该通道
    _redis.unsubscribe(user.getId());
    // 更新用户的状态为offline
    if(user.getId() != -1){
        user.setState("offline");
        _userModel.updateState(user);
    }
    return ;
}

// 服务器异常，业务重置
void ChatService::reset(){
    //将所有online用户都设置为离线模式offline
    _userModel.resetState();  
}

// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid){
    auto it = _msgHandlerMap.find(msgid);
    if(it == _msgHandlerMap.end())  //发生错误
    {
        // 返回一个默认的处理器：空操作
        return [=](const TcpConnectionPtr& conn,json& js,Timestamp){
            LOG_ERROR << "msgid = "<< msgid << " cannot find handler!";
        };
    }
    else {
        return _msgHandlerMap[msgid];
    }
}

// 从redis消息队列中获取订阅的消息 （回调函数专用）
void ChatService::handlerRedisSubscribeMessage(int userid,string msg){
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if(it != _userConnMap.end()){
        it->second->send(msg);
        return;
    }

    _offlineMsgModel.insert(userid,msg);
}

