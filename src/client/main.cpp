#include "public.hpp"
#include "group.hpp"
#include "user.hpp"
#include "json.hpp"

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <ctime>
#include <string.h>
#include <semaphore.h>   
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <atomic>


using namespace std;
using json = nlohmann::json;

/*全局变量*/ 
// 记录当前系统登陆的用户信息
User g_currentUser;
// 记录当前系统登录用户的好友列表信息
vector<User> g_currentUserFriend;
// 记录当前系统登录用户的群组列表信息
vector<Group> g_currentUserGroup;
// 控制主菜单操作
bool isMainMenuRunning = false;
// 用于读写线程之间的通信
sem_t rwsem;
// 记录登录状态
atomic_bool g_isLoginSuccess{false};

/* 各类函数声明 */
// 各类功能函数 声明
void help(int fd = 0,string str = "");
void chat(int cfd,string str);
void addfriend(int cfd,string str);
void creategroup(int cfd,string str);
void addgroup(int cfd,string str);
void groupchat(int cfd,string str);
void loginout(int cfd,string str = "");

/* 两个map */
// 客户端命令列表
unordered_map<string,string> commandMap = {
    {"help","显示所有支持的命令,格式: help"},
    {"chat","一对一聊天,格式: chat:friendid:message"},
    {"addfriend","添加好友,格式: addfriend:friendid"},
    {"creategroup","创建群组,格式: creategroup:groupname:groupdesc"},
    {"addgroup","加入群组,格式: addgroup:groupid"},
    {"groupchat","群聊天,格式: groupchat:groupid:message"},
    {"loginout","注销,格式: loginout"}
};
// 客户端命令处理
unordered_map<string,function<void(int,string)>> commandHandlerMap = {
    {"help",help},
    {"chat",chat},
    {"addfriend",addfriend},
    {"creategroup",creategroup},
    {"addgroup",addgroup},
    {"groupchat",groupchat},
    {"loginout",loginout}
};


/* 函数声明 */
// 接收线程
void readTaskHandler(int cfd);
// 获取系统时间
string getCurrentTime();
// 主聊天界面
void mainMenu(int cfd);
// 显示当前登录成功用户的基本信息
void showCurrentUserData();

/* 各类业务处理逻辑函数*/
// 处理登录的响应逻辑
void doLoginResponse(json& responsejs);
// 处理注册的响应逻辑
void doRegResponse(json& responsejs);

// main会作为发送线程
int main(int argc,char** argv){
    if(argc < 3){
        cerr << "command invalid! example:./ChatClient ServerIp ServerPort" <<endl;
        exit(-1);
    }
    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // 创建客户端连接套接字
    int cfd = socket(AF_INET,SOCK_STREAM,0);
    if(cfd == -1){
        cerr << "socket create error" << endl;
        exit(-1);
    }

    // 连接服务器
    sockaddr_in server;
    memset(&server,0,sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    //
    if(-1 == connect(cfd,(sockaddr*)&server,sizeof(sockaddr_in))){
        cerr << "connect server error " << endl;
        exit(-1);
    }

    // 初始化读写线程通信用的信号量
    sem_init(&rwsem,0,0);
    std::thread readTask(readTaskHandler,cfd);  // 创建用于接收的子线程
    readTask.detach();   // 线程分离

    // main本线程用于接受用户输入，发送数据
    while(1){
        // 显示首页 登录、注册、退出
        cout <<"===========================\n";
        cout <<"1. login" << endl;
        cout <<"2. resgister" << endl;
        cout <<"3. quit" << endl;
        cout <<"===========================\n";
        cout << "choice:";
        int choice = 0;
        cin >> choice;
        cin.get();   // 读走缓冲区残留的回车

        switch(choice){
            case 1:  // 登录
            {
                int id = 0;
                char pwd[50] = {0};
                cout<<"userid:";
                cin >> id;
                cin.get();   // 读走缓冲区残留的回车
                cout<<"userpassward:";
                cin.getline(pwd,50);

                json js;
                js["msgid"] = LOGIN_MSG;
                js["id"] = id;
                js["passward"] = pwd;
                string request = js.dump();

                g_isLoginSuccess = false;
                // 发送数据
                int len = send(cfd,request.c_str(),strlen(request.c_str())+1,0);
                if(len == -1){
                    cerr << "send login msg error:"<<request<<endl;
                }
                // 等待信号量，由子线程处理完登录的响应信息之后，通知此处
                sem_wait(&rwsem);

                if(g_isLoginSuccess){
                    // 显示首页
                    isMainMenuRunning = true;
                    mainMenu(cfd);
                }// 登录失败在子线程会输出错误信息的
                break;
            }
            case 2:  // 注册
            {    
                char name[50] = {0};
                char pwd[50] = {0};
                cout<<"username:";
                cin.getline(name,50);
                cout << "userpassward:";
                cin.getline(pwd,50);

                json js;
                js["msgid"] = REG_MSG;
                js["name"] = name;
                js["passward"] = pwd;
                string request = js.dump();

                int len = send(cfd,request.c_str(),strlen(request.c_str())+1,0);
                if(len == -1){
                    cerr << "send register msg error:"<<request<<endl;
                }
                // 等待信号量，由子线程处理完注册的响应信息之后，通知此处
                sem_wait(&rwsem);
                break;
            }

            case 3:
                close(cfd);
                sem_destroy(&rwsem);
                exit(0);
                // break;
            default:
                cout << "invalid input! please input again!"<<endl;
                break;
        }
    }
    return 0;
}

// 处理登录响应
void doLoginResponse(json& responsejs){
    if(0 != responsejs["errno"].get<int>()) // 登录失败，有错误
    {
        cerr << responsejs["errmsg"] << endl;
        g_isLoginSuccess = false;
    }
    else{
        // 登录成功
        // 记录当前用户的信息
        g_currentUser.setId(responsejs["id"].get<int>());
        g_currentUser.setName(responsejs["name"]);
        // 记录其好友信息
        if(responsejs.contains("friends")){
            g_currentUserFriend.clear();
            vector<string> vec = responsejs["friends"];
            for(string& str : vec){
                json js = json::parse(str);
                User user;
                user.setId(js["id"].get<int>());
                user.setName(js["name"]);
                user.setState(js["state"]);
                g_currentUserFriend.push_back(user);
            }
        }

        // 记录其群组信息
        if(responsejs.contains("groups")){
            g_currentUserGroup.clear();
            vector<string> vec = responsejs["groups"];
            for(string& gstr : vec){
                json js = json::parse(gstr);
                Group group;
                group.setId(js["id"].get<int>());
                group.setGroupName(js["groupname"]);
                group.setGroupDesc(js["groupdesc"]);

                vector<string> vec2 = js["users"];
                for(string& ustr : vec2){
                    GroupUser user;
                    json ujs = json::parse(ustr);
                    user.setId(ujs["id"].get<int>());
                    user.setName(ujs["name"]);
                    user.setState(ujs["state"]);
                    user.setRole(ujs["role"]);
                    group.getGroupUser().push_back(user);
                }
                g_currentUserGroup.push_back(group);
            }
        }
        // 显示当前登录用户的信息
        showCurrentUserData();

        //  显示当前登录用户的离线信息:个人聊天信息或群组信息
        if(responsejs.contains("offlinemsg")){
            cout<<"离线消息："<<endl;
            vector<string> vec = responsejs["offlinemsg"];
            for(string& str : vec){
                json js = json::parse(str);
                if(ONE_CHAT_MSG == js["msgid"].get<int>())  //好友聊天信息
                {
                    // 格式: time[id]name said: msg
                    cout << js["time"].get<string>() << " [" <<js["id"] << "]" << js["name"].get<string>()
                            <<" said: " <<js["msg"].get<string>() <<endl;
                }
                else // 群组信息
                {
                    // 格式: 群消息[groupid]:time[id]name said: msg
                    cout << "群消息[" <<js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] <<"]" <<js["name"].get<string>()
                            <<" said: " <<js["msg"].get<string>() <<endl;
                
                }
            }
        }
        g_isLoginSuccess = true;
    }
    return ;
}
// 处理注册响应
void doRegResponse(json& responsejs){
    if(0 != responsejs["errno"].get<int>()) // 注册失败，有错误
    {
        // 注册是只有这一个错误
        cerr << "name is already exit,register fail!" << endl;
    }
    else{
        // 注册成功
        cout << "name register success,userid is " << responsejs["id"]
            << ", do not forget it!" <<endl;
    }
    return ;
}

// 接收线程
void readTaskHandler(int cfd){
    while(1){
        char buffer[1024] = {0};
        int len = recv(cfd,buffer,1024,0);    // 会阻塞的
        if(-1 == len || 0 == len) // 出错啦
        {
            close(cfd);
            exit(-1);
        }
        // 接收到server发送来的数据
        json js = json::parse(buffer);
        int msgtype = js["msgid"].get<int>();  // 获取消息类型
        // 这里将例程的if改成了switch
        switch(msgtype){
            case ONE_CHAT_MSG:   // 好友的聊天信息
                cout<< js["time"].get<string>() << "[" <<js["id"] <<"]"
                    << js["name"].get<string>()
                    << " said: " << js["msg"].get<string>() <<endl;
                break;
            case GROUP_CHAT_MSG :  // 群聊天信息
                cout<< "群消息:["<<js["groupid"] <<"]:"
                    << js["time"].get<string>() << "[" <<js["id"] <<"]"
                    << js["name"].get<string>()
                    << " said: " << js["msg"].get<string>() <<endl;
                break;
            case LOGIN_MSG_ACK:   //登录消息响应
                doLoginResponse(js);// 处理登录响应的业务逻辑
                sem_post(&rwsem);// 通知主线程，登录结果处理完成
                break;
            case REG_MSG_ACK:     //注册消息响应
                doRegResponse(js);// 处理注册响应的业务逻辑
                sem_post(&rwsem);// 通知主线程，注册结果处理完成
                break;
        }
    }
    return ;
}

// 获取系统时间 (聊天时需要添加时间信息)
string getCurrentTime(){
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm* ptm = localtime(&tt);
    char data[60] = {0};
    sprintf(data,"%d-%02d-%02d %02d:%02d:%02d",
                (int)ptm->tm_year+1990,(int)ptm->tm_mon+1,(int)ptm->tm_mday,
                (int)ptm->tm_hour,(int)ptm->tm_min,(int)ptm->tm_sec);
    return string(data);
}

// 主聊天界面
void mainMenu(int cfd){
    help();

    char buffer[1024] = {0};
    while(isMainMenuRunning){
        cin.getline(buffer,1024);   // 输入
        string commandbuf(buffer);
        string command;   // 存储命令
        int idx = commandbuf.find(":");
        if(-1 == idx){
            command = commandbuf;    
        }
        else{
            command = commandbuf.substr(0,idx);
        }

        // 
        auto it = commandHandlerMap.find(command);
        if(it == commandHandlerMap.end()){
            cerr << "invalid input command " << endl;
            continue;
        }
        else{
            // 调用相应的功能回调函数 (作用：mainMenu对修改封闭，添加或修改功能不需要修改这个函数)
            it->second(cfd,commandbuf.substr(idx+1,commandbuf.size()-idx));
        }
    }
    return;
}

// 显示当前登录成功用户的基本信息
void showCurrentUserData(){
    cout<<"===============login user=============="<<endl;
    cout<<"current login user \nid:"<<g_currentUser.getId() << " \nname:"<<g_currentUser.getName() <<endl;
    
    cout<<"---------------friend list------------"<<endl;
    if(!g_currentUserFriend.empty()){
        cout<<"id name state"<<endl;
        for(User& user : g_currentUserFriend){
            cout<<user.getId()<<" "<<user.getName()<<" "<<user.getState()<<endl;
        }
    }
    else{
        cout<<"empty!"<<endl;
    }

    cout<<"---------------group list------------"<<endl;
    if(!g_currentUserGroup.empty()){
        cout<<"id name descrition"<<endl;
        for(Group& group : g_currentUserGroup){
            cout<<group.getId()<<" "<<group.getGroupName()<<" "<<group.getGroupDesc()<<endl;
            for(GroupUser& guser : group.getGroupUser()){
                cout<<"\b" << guser.getId() << " " << guser.getName() << " " << guser.getState() << " "<< guser.getRole() <<endl;
            }
        }
    }
    else{
        cout<<"empty!"<<endl;
    }
    cout << "=============================" << endl;
}



// 各类功能函数 实现:编写json，发送给server；或是与客户交互
void help(int fd ,string str){
    cout<<"show command list >>> "<<endl;
    for(auto& p : commandMap){
        cout << p.first << " : " << p.second << endl;
    }
    cout<<endl;
    return ;
}
void chat(int cfd ,string str){
    // str 格式: friendid:message
    int idx = str.find(":");   // 找到分割的地方
    if (-1 == idx){
        cerr << "chat command invalid!" << endl;
        return ;
    }
    int friendid = atoi(str.substr(0,idx).c_str());
    string message = str.substr(idx+1,str.size()-idx);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buf = js.dump();

    // 发送给cfd
    int len = send(cfd,buf.c_str(),strlen(buf.c_str())+1,0);
    if(-1 == len){
        cerr << "send chat msg failed -> " << buf << endl;
    }
    return ;
}
void addfriend(int cfd ,string str){
    // str 格式: friendid
    int friendid = atoi(str.c_str());

    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    // js["name"] = g_currentUser.getName();
    // js["time"] = getCurrentTime();
    string buf = js.dump();

    // 发送给cfd
    int len = send(cfd,buf.c_str(),strlen(buf.c_str())+1,0);
    if(-1 == len){
        cerr << "send addfriend msg failed -> " << buf << endl;
    }
    return ;
}
void creategroup(int cfd ,string str) {
    // str 格式: groupname:groupdesc
    int idx = str.find(":");   // 找到分割的地方
    if (-1 == idx){
        cerr << "creategroup command invalid!" << endl;
        return ;
    }
    string groupname = str.substr(0,idx);
    string groupdesc = str.substr(idx+1,str.size()-idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    // js["name"] = g_currentUser.getName();
    // js["time"] = getCurrentTime();
    string buf = js.dump();

    // 发送给cfd
    int len = send(cfd,buf.c_str(),strlen(buf.c_str())+1,0);
    if(-1 == len){
        cerr << "send creategroup msg failed -> " << buf << endl;
    }
    return ;
}
void addgroup(int cfd ,string str){
    // str 格式: friendid
    int groupid = atoi(str.c_str());

    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    // js["name"] = g_currentUser.getName();
    // js["time"] = getCurrentTime();
    string buf = js.dump();

    // 发送给cfd
    int len = send(cfd,buf.c_str(),strlen(buf.c_str())+1,0);
    if(-1 == len){
        cerr << "send addgroup msg failed -> " << buf << endl;
    }
    return ;
}
void groupchat(int cfd,string str){
    // str 格式: groupid:message
    int idx = str.find(":");   // 找到分割的地方
    if (-1 == idx){
        cerr << "groupchat command invalid!" << endl;
        return ;
    }
    int groupid = atoi(str.substr(0,idx).c_str());
    string message = str.substr(idx+1,str.size()-idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buf = js.dump();

    // 发送给cfd
    int len = send(cfd,buf.c_str(),strlen(buf.c_str())+1,0);
    if(-1 == len){
        cerr << "send groupchat msg failed -> " << buf << endl;
    }
    return ;
}
void loginout(int cfd,string str){
    // str 格式: ""
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();
    // js["name"] = g_currentUser.getName();
    // js["time"] = getCurrentTime();
    string buf = js.dump();

    // 发送给cfd
    int len = send(cfd,buf.c_str(),strlen(buf.c_str())+1,0);
    if(-1 == len){
        cerr << "send loginout msg failed -> " << buf << endl;
    }
    else{
        // 要退出主菜单了
        isMainMenuRunning = false;
    }
    return ;
}