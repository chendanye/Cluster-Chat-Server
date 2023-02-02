#ifndef __USER_H__
#define __USER_H__

#include <iostream>
#include <string>
using namespace std;

//对应mysql数据库chat_server中的User表
class User{
public:
    User(int id = -1,string name = "", string passward = "",string state = "offline")
    {
        this->id = id;
        this->name = name;
        this->passward = passward;
        this->state = state;
    }

    // 四个属性设置接口
    void setId(int id){this->id = id;}
    void setName(string name){this->name = name;}
    void setPwd(string passward){this->passward = passward;}
    void setState(string state){this->state = state;}

    //四个属性获取接口
    int getId(){return this->id;}
    string getName(){return this->name;}
    string getPwd(){return this->passward;}
    string getState(){return this->state;}

private:
    int id;
    string name;
    string passward;
    string state;    
    // 不用枚举而用string跟后面传信息有关系 用枚举麻烦
};


#endif