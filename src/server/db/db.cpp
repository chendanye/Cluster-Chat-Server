#include "db.hpp"
#include <muduo/base/Logging.h>
#include <iostream>

std::string ip = "127.0.0.1";         //mysql的ip地址
unsigned int port = 3306;             //mysql的端口号
std::string user = "root";            //mysql用户名
std::string passward = "root";        //mysql密码
std::string dbname = "chat_server";   //数据库名

// 初始化数据库
MySQL::MySQL(){
    _conn = mysql_init(nullptr);  //初始化一个MYSQL对象
    if(_conn == nullptr){  //出错
        std::cout<<"mysql_init error"<<std::endl;
    }
    return;
}

// 释放数据库
MySQL::~MySQL(){
    if(_conn != nullptr)
        mysql_close(_conn);
    _conn = nullptr;
    return;
}

// 连接数据库操作
bool MySQL::connect(){
    //发起连接
    MYSQL* p = mysql_real_connect(_conn,ip.c_str(),user.c_str(),passward.c_str(),dbname.c_str(),port,nullptr,0);
    if(p == nullptr){
        LOG_INFO << "connect mysql fail !";
        std::cout<<"mysql_real_connect error"<<std::endl;
        
        return false;
    }
    //C/C++代码默认是ASCII，如果不设置，从mysql拉下来的中文显示会乱码
    mysql_query(_conn,"set names gbk");
    LOG_INFO << "connect mysql success !";
    return true;
}

// 更新操作 （增删改）
bool MySQL::update(std::string sql){
    if(_conn == nullptr) return false;
    int ret = mysql_query(_conn,sql.c_str());
    if(ret != 0){
        LOG_INFO << __FILE__<<":"<<__LINE__<<" : "<<sql<<"  fail!";
        std::cout<<"mysql_query error! sql: "<<sql<<std::endl;
        return false;
    }
    return true;
}

// 查询操作
MYSQL_RES* MySQL::query(std::string sql){
    if(_conn == nullptr) return nullptr;
    int ret = mysql_query(_conn,sql.c_str());
    if(ret != 0){
        LOG_INFO << __FILE__<<":"<<__LINE__<<" : "<<sql<<" query fail!";
        std::cout<<"mysql_query error! sql: "<<sql<<std::endl;
        return nullptr;
    }
    //#考虑换成mysql_store_result()?
    return mysql_use_result(_conn);
    //  一旦完成了对结果集的操作，必须调用mysql_free_result()！！
}

// 获取连接
MYSQL* MySQL::getConnection(){
    return this->_conn;
}