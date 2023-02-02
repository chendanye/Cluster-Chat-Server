#ifndef __DB_H__
#define __DB_H__

#include <mysql/mysql.h>
#include <string>

class MySQL{
public:
    // 初始化数据库
    MySQL();
    // 释放数据库
    ~MySQL();
    // 连接数据库操作
    bool connect();
    // 更新操作 （增删改）
    bool update(std::string sql);
    // 查询操作
    MYSQL_RES* query(std::string sql);
    // 获取连接
    MYSQL* getConnection();
private:
    MYSQL* _conn;
};


#endif
