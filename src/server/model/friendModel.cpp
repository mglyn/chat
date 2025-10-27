#include "friendModel.hpp"
#include "db.hpp"

#include <algorithm>

bool FriendModel::insert(int id1, int id2){
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "insert into Friend values(%d, %d)", id1, id2);

    MySQL mysql;
    return mysql.connect() && mysql.update(sql);
}

std::vector<User> FriendModel::query(int id){
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "select a.id, a.name, a.state from User a inner join Friend b on b.friendid = a.id where b.userid = %d", id);

    MySQL mysql;
    std::vector<User> vec;
    if(mysql.connect()){
        if(MYSQL_RES* res = mysql.query(sql)){
            while(MYSQL_ROW row = ::mysql_fetch_row(res)){
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}