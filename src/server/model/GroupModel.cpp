#include "GroupModel.hpp"
#include "db.hpp"
#include "user.hpp"
#include <string>

using namespace std;

//创建群组
bool GroupModel::CreateGroup(Group& group){
    string sql = "insert into AllGroup(groupname,groupdesc) values(\'" + group.getGroupName() + "\',\'" + group.getGroupDesc() + "\')";
    MySQL mysql;
    if(mysql.connect()){
        if(mysql.update(sql.c_str())){
            // 将插入数据库自动生成的id设置回去
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

//将成员加入群组
bool GroupModel::AddGroup(int userid,int groupid,string role){
    string sql = "insert into GroupUser values(" + to_string(groupid) + "," + to_string(userid) + ",\'" + role + "\')";
    MySQL mysql;
    if(mysql.connect()){
        if(mysql.update(sql.c_str())){
            return true;
        }
    }
    return false;
}

//查询用户所在群组信息
vector<Group> GroupModel::QueryGroups(int userid){
    string sql = "select a.id,a.groupname,a.groupdesc from AllGroup a \
    inner join GroupUser b on a.id = b.groupid where b.userid = " + to_string(userid);
    MySQL mysql;
    vector<Group> vec;
    if(mysql.connect()){
        MYSQL_RES * res = mysql.query(sql.c_str());
        if( res != nullptr){
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr){
                Group group;
                group.setId(atoi(row[0]));
                group.setGroupName(row[1]);
                group.setGroupDesc(row[2]);
                vec.push_back(group);
            }
            mysql_free_result(res);
        }
    }
    //
    for(Group& group : vec){
        sql = "select a.id,a.name,a.state,b.grouprole from User a \
        inner join GroupUser b on b.userid = a.id where b.groupid = " + to_string(group.getId());
        MYSQL_RES *res = mysql.query(sql.c_str());
        if(res != nullptr){
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr){
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getGroupUser().push_back(user);
            }
            mysql_free_result(res);
        }
        return vec;
    }

    return vec;
}

//根据指定的groupid信息查询同一个群组的其他成员信息(用于在群聊中发消息)
vector<int> GroupModel::queryGroupUsers(int userid,int groupid){
    string sql = "select userid from GroupUser where groupid = " + to_string(groupid) + " and userid != " + to_string(userid);
    MySQL mysql;
    vector<int> vec;
    if(mysql.connect()){
        MYSQL_RES *res = mysql.query(sql.c_str());
        if(res != nullptr){
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res))!= nullptr){
                vec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}

