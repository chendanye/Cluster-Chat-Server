# MySQL数据库

1. 库chat_server

2. 5个表

   **User**表

   | 字段名称 | 字段类型                 | 说明               | 约束                         |
   | -------- | ------------------------ | ------------------ | ---------------------------- |
   | id       | int                      | 用户id             | primary key 、auto_increment |
   | name     | VARCHAE（50）            | 用户名(相当于账号) | not null、unique             |
   | passward | VARVHAR（50）            | 密码               | not null                     |
   | state    | ENUM('online','offline') | 当前登陆状态       | default 'offline'            |

   **Friend**表

   | 字段名称 | 字段类型 | 说明     | 约束                     |
   | -------- | -------- | -------- | ------------------------ |
   | userid   | int      | 用户id   | not null 联合primary key |
   | friendid | int      | 好友的id | not null 联合primary key |

   **AllGroup**表

   | 字段名称  | 字段类型       | 说明         | 约束                         |
   | --------- | -------------- | ------------ | ---------------------------- |
   | id        | int            | 群组id       | primary key 、auto_increment |
   | groupname | VARCHAE（50）  | 群组名       | not null、unique             |
   | groupdesc | VARCHAR（200） | 群组功能描述 | default ''                   |

   **GroupUser**表

   | 字段名称  | 字段类型                 | 说明     | 约束               |
   | --------- | ------------------------ | -------- | ------------------ |
   | groupid   | int                      | 群组id   | not null、联合主键 |
   | userid    | int                      | 成员id   | not null、联合主键 |
   | grouprole | enum('creator','normal') | 组内角色 | default 'normal'   |

   **OfflineMessage**表

   | 字段名称 | 字段类型    | 说明                       | 约束     |
   | -------- | ----------- | -------------------------- | -------- |
   | userid   | int         | 用户id                     | not null |
   | message  | varchar(50) | 离线消息（存储json字符串） | not null |

   

# 相关代码：

```sql
-- 登录 
mysql -u root -p pwd   

-- 创建数据库chat_server
create database chat_server;   

use chat_server;

-- 创建User表
create table User(         
	id int primary key auto_increment,
    name varchar(50) not null unique,
    passward varchar(50) not null,
    state enum('online','offline') default 'offline'
);
-- 创建Friend表
create table Friend(       
	userid int not null,
    friendid int not null,
    primary key(userid,friendid)  -- 联合主键
);
-- 创建AllGroup表
create table AllGroup(     
    id int primary key auto_increment,
    groupname varchar(50) not null unique,
    groupdesc varchar(200) default''
);

-- 创建GroupUser表
create table GroupUser(    
    groupid int not null,
    userid int not null ,
    grouprole enum('creator','normal') default 'normal',
    primary key(groupid,userid)
);

-- 创建OfflineMessage表
create table OfflineMessage(
	userid int not null,
    message varchar(50) not null
);
```







