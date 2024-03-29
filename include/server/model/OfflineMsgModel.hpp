#ifndef __OFFLINE_MESSAGE_MODEL_H__
#define __OFFLINE_MESSAGE_MODEL_H__

#include <string>
#include <vector>
using namespace std;

class OfflineMsgModel{
public:
    //存储用户的离线信息
    void insert(int userid,string msg);
    //删除用户的离线信息
    void remove(int userid);
    //查询用户的离线信息
    vector<string> query(int userid);
};

#endif