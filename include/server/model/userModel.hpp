#pragma once

#include "user.hpp"

#include <optional>

// User表的操作类
class UserModel{
public:
    bool insert(User& user);
    std::optional<User> query(int id);
    bool updateState(int id, std::string state);
    bool resetAllState();
private:

};
