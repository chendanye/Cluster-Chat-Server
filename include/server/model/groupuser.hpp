#ifndef __GROUPUSER_H__
#define __GROUPUSER_H__

#include "user.hpp"
#include <string>

using namespace std;

//注意：这个不是对应mysql的GroupUser表
//群组用户 直接继承User（只是多了一个role属性）
class GroupUser : public User{
public:
    void setRole(string role){this->role = role;}
    string getRole(){return this->role;}

private:
    string role;
    //另外的属性是：id name passward state
};

#endif