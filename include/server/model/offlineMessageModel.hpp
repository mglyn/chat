#pragma once

#include <string>
#include <vector>

class OfflineMsgModel{

public:

    bool insert(int id, std::string msg);

    bool clear(int id);

    std::vector<std::string> query(int id);
};