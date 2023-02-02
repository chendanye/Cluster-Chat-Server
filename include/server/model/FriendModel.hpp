#ifndef __FRIEND_MODEL_H__
#define __FRIEND_MODEL_H__

#include "user.hpp"
#include <vector>

//维护对Frien表操作的接口
class FriendModel{
public:
    //添加好友关系
    void insert(int userid,int friendid);
    //返回一个user的好友列表
    vector<User> queryFriends(int userid);
};

#endif