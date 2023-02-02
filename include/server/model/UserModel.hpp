#ifndef __USER_MODEL_H__
#define __USER_MODEL_H__

#include "user.hpp"
#include <string>

//维护对User表操作的接口
class UserModel{
public:
    //User表的添加
    bool insert(User& user);
    //根据用户id查询用户信息
    User queryUser(int userid);
    //更新用户的状态信息
    bool updateState(User user);
    //重置全部用户的状态信息   (重置为offline)
    void resetState();
};

#endif