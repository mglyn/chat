#include "offlineMessageModel.hpp"

#include "db.hpp"
#include "server/db/mysql_conn_guard.hpp"

bool OfflineMsgModel::insert(int id, std::string msg){
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "insert into OfflineMessage(userid, message) values(%d, '%s')", id, msg.c_str());
    
    MySQLConnGuard guard;
    MYSQL* conn = guard.get();
    if (!conn) return false;
    return mysql_query(conn, sql) == 0;
}

bool OfflineMsgModel::clear(int id){
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "delete from OfflineMessage where userid = %d", id);
    
    MySQLConnGuard guard;
    MYSQL* conn = guard.get();
    if (!conn) return false;
    return mysql_query(conn, sql) == 0;
}

std::vector<std::string> OfflineMsgModel::query(int id){
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "select message from OfflineMessage where userid = %d", id);
    
    MySQLConnGuard guard;
    MYSQL* conn = guard.get();
    std::vector<std::string> vec;
    if (!conn) return vec;
    if (mysql_query(conn, sql) == 0) {
        MYSQL_RES* res = mysql_use_result(conn);
        if (res) {
            while (MYSQL_ROW row = ::mysql_fetch_row(res)) {
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}