#include "UserModel.hpp"
#include "db.hpp"
#include "user.hpp"
#include <iostream>
#include <string>

using namespace std;

//User表的添加
bool UserModel::insert(User& user){
    string sql = "insert into User(name,passward,state) values(\'"+ user.getName() + "\',\'" + user.getPwd() + "\',\'" + user.getState() + "\')";
    MySQL mysql;
    if(mysql.connect()){
        if(mysql.update(sql.c_str())){
            //mysql_insert_id() 返回上一次insert操作产生的id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;

}

//根据用户id查询用户信息
User UserModel::queryUser(int userid){
    string sql = "select * from User where id = " + to_string(userid);
    MySQL mysql;
    User user;     // User user(); 可能会被编译器当成是一个函数声明而报错
    if(mysql.connect()){
        MYSQL_RES* res = mysql.query(sql.c_str());
        if(res != nullptr){
            MYSQL_ROW row = mysql_fetch_row(res);
            if(row != nullptr){
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    }
    return user;   //这个的id可是-1
}

//更新用户的状态信息
bool UserModel::updateState(User user){
    string sql = "update User set state = \'" + user.getState() + "\' where id = " + to_string(user.getId());
    MySQL mysql;
    if(mysql.connect()){
        if(mysql.update(sql.c_str())){
            return true;
        }
        else{
            cerr << "updateState failed! sql : " << sql <<endl;
        }
    }
    return false;
}

//重置全部用户的状态信息   (重置为offline)
void UserModel::resetState(){
    string sql = "update User set state = \'offline\' where state = \'online\'" ;
    MySQL mysql;
    if(mysql.connect()){
        if(mysql.update(sql.c_str())){
            return ;
        }
    }
    return ;
}

