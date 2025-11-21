
#include "friendModel.hpp"
#include "db.hpp"
#include "mysql_conn_guard.hpp"
#include <algorithm>

bool FriendModel::insert(int id1, int id2){
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "insert into Friend values(%d, %d)", id1, id2);

    MySQLConnGuard guard;
    MYSQL* conn = guard.get();
    if (!conn) return false;
    if (mysql_query(conn, sql) == 0) {
        return true;
    }
    return false;
}

std::vector<User> FriendModel::query(int id){
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "select a.id, a.name, a.state from User a inner join Friend b on b.friendid = a.id where b.userid = %d", id);

    MySQLConnGuard guard;
    MYSQL* conn = guard.get();
    std::vector<User> vec;
    if (!conn) return vec;
    if (mysql_query(conn, sql) == 0) {
        MYSQL_RES* res = mysql_use_result(conn);
        if (res) {
            while (MYSQL_ROW row = ::mysql_fetch_row(res)) {
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