#include "OfflineMsgModel.hpp"
#include "db.hpp"


//存储用户的离线信息
void OfflineMsgModel::insert(int userid,string msg){
    // 组合sql语句
    string sql = "insert into OfflineMessage values(" + to_string(userid) + ",\'" + msg + "\')";

    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql.c_str());
    }
}
//删除用户的离线信息
void OfflineMsgModel::remove(int userid){
    string sql = "delete from OfflineMessage where userid = " + to_string(userid) ;
    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql.c_str());
    }
}
//查询用户的离线信息
vector<string> OfflineMsgModel::query(int userid){
    string sql = "select message from OfflineMessage where userid = " + to_string(userid) ;
    MySQL mysql;
    vector<string> ret;
    if(mysql.connect()){
        MYSQL_RES* res = mysql.query(sql.c_str());
        if(res != nullptr){
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                ret.push_back(row[0]);
            }
            mysql_free_result(res);
            return ret;
        }
    }
    return ret;   //这个ret是空的
}

