#pragma once

#include <vector>

#include "user.hpp"

class FriendModel{

public:

    // 添加好友关系
    bool insert(int id1, int id2);

    // 返回好友列表
    std::vector<User> query(int id);
};