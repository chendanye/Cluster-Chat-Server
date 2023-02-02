#ifndef __GROUP_MODEL_H__
#define __GROUP_MODEL_H__

#include "group.hpp"
#include <string>

using namespace std;

//维护对AllGroup、GroupUser表操作的接口
class GroupModel{
public:
    //创建群组
    bool CreateGroup(Group& group);
    //加成员加入群组
    bool AddGroup(int userid,int groupid,string role);
    //查询用户所在群组信息
    vector<Group> QueryGroups(int userid);
    //根据指定的groupid信息查询同一个群组的其他成员信息(用于在群聊中发消息)
    vector<int> queryGroupUsers(int userid,int groupid);
};

#endif