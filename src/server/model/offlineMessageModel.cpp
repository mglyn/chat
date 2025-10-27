#include "offlineMessageModel.hpp"

#include "db.hpp"

bool OfflineMsgModel::insert(int id, std::string msg){
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "insert into OfflineMessage(userid, message) values(%d, '%s')", id, msg.c_str());
    
    MySQL mysql;
    return mysql.connect() && mysql.update(sql);
}

bool OfflineMsgModel::clear(int id){
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "delete from OfflineMessage where userid = %d", id);
    
    MySQL mysql;
    return mysql.connect() && mysql.update(sql);
}

std::vector<std::string> OfflineMsgModel::query(int id){
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "select message from OfflineMessage where userid = %d", id);
    
    MySQL mysql;
    std::vector<std::string> vec;
    if(mysql.connect()){
        if(MYSQL_RES* res = mysql.query(sql)){
            while(MYSQL_ROW row = ::mysql_fetch_row(res)){
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}