
#include "userModel.hpp"
#include "server/db/mysql_conn_guard.hpp"

bool UserModel::insert(User& user){
    // 组装sql语句
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "insert into User(name, password, state) values('%s', '%s', '%s')", 
        user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());
    
    MySQLConnGuard guard;
    MYSQL* conn = guard.get();
    if (!conn) return false;
    if (mysql_query(conn, sql) == 0) {
        user.setId(::mysql_insert_id(conn));
        return true;
    }
    return false;
}

std::optional<User> UserModel::query(int id){
// 组装sql语句
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "select * from User where id = %d", id);
    
    MySQLConnGuard guard;
    MYSQL* conn = guard.get();
    User user;
    if (!conn) return {};
    if (mysql_query(conn, sql) == 0) {
        MYSQL_RES* res = mysql_use_result(conn);
        if (res) {
            if (MYSQL_ROW row = ::mysql_fetch_row(res)) {
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return std::make_optional<User>(std::move(user));
            }
            mysql_free_result(res);
        }
    }
    return {};
} 

bool UserModel::updateState(int id, std::string state){
    // 组装sql语句
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "update User set state = '%s' where id = %d", state.c_str(), id);
    MySQLConnGuard guard;
    MYSQL* conn = guard.get();
    if (!conn) return false;
    return mysql_query(conn, sql) == 0;
}

bool UserModel::resetAllState(){
    char sql[1024] = {"update User set state = 'offline' where state != 'offline'"};
    MySQLConnGuard guard;
    MYSQL* conn = guard.get();
    if (!conn) return false;
    return mysql_query(conn, sql) == 0;
}