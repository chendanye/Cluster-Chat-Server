#ifndef __PUBLIC_H__
#define __PUBLIC_H__

//Server 与 Client 的公共文件

//消息类型
enum EnMsgType{
    LOGIN_MSG = 1,   //登录消息
    LOGIN_MSG_ACK,   //登录消息响应
    LOGINOUT_MSG,    //注销消息(就是退出登录)

    REG_MSG,         //注册消息
    REG_MSG_ACK,     //注册消息响应

    ONE_CHAT_MSG,    //聊天消息
    ADD_FRIEND_MSG,  //添加好友消息

    CREATE_GROUP_MSG,  //创建群组
    ADD_GROUP_MSG,     //加入群组
    GROUP_CHAT_MSG,    //群组聊天
};

// 错误号
enum ERRNOS
{
    NO_ERROR = 0,    // 没有错误
    ERROR_REGISTER,  // 注册错误  
    ERROR_LOGIN,     // 登录错误
};

#endif