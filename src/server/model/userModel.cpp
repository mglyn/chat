#include "userModel.hpp"
#include "db.hpp"

bool UserModel::insert(User& user){
    // 组装sql语句
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "insert into User(name, password, state) values('%s', '%s', '%s')", 
        user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());
    
    MySQL mysql;
    if(mysql.connect()){
        if(mysql.update(sql)){
            // 获得主键
            user.setId(::mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }

    return false;
}

std::optional<User> UserModel::query(int id){
// 组装sql语句
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "select * from User where id = %d", id);
    
    MySQL mysql;
    User user;
    if(mysql.connect()){
        if(MYSQL_RES* res = mysql.query(sql)){
            if(MYSQL_ROW row = ::mysql_fetch_row(res)){
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
    MySQL mysql;
    return mysql.connect() && mysql.update(sql);
}

bool UserModel::resetAllState(){
    char sql[1024] = {"update User set state = 'offline' where state != 'offline'"};
    MySQL mysql;
    return mysql.connect() && mysql.update(sql);
}