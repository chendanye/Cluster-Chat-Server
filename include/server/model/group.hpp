#ifndef __ALLGROUP_H__
#define __ALLGROUP_H__

#include "groupuser.hpp"
#include <string>
#include <vector>

using  namespace std;

//对应mysql中的chat_server数据库AllGroup表 结合GroupUser.hpp对应表GroupUser
class Group{
public:
    Group(int id = -1,string name = "",string desc = "")
    {
        this->id = id;
        this->groupname = name;
        this->groupdesc = desc;
    }
    
    //设置函数
    void setId(int id){this->id = id;}
    void setGroupName(string name){this->groupname = name;}
    void setGroupDesc(string desc){this->groupdesc = desc;}

    //获取函数
    int getId(){return this->id;}
    string getGroupName(){return this->groupname;}
    string getGroupDesc(){return this->groupdesc;}
    
    vector<GroupUser>& getGroupUser(){return this->users;}

private:
    int id;
    string groupname;    
    string groupdesc;    
    vector<GroupUser> users;   //群组的成员集合
};


#endif