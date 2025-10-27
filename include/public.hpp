#pragma once

// server 和 client 公共头

enum EnMsgType{
    MSG_LOGIN = 1,
    MSG_ACK_LOGIN,
    MSG_LOGOUT,
    MSG_REG,
    MSG_ACK_REG,
    MSG_CHAT,
    MSG_ADD_FRIEND,

    MSG_CREATE_GROUP,
    MSG_ADD_GROUP,
    MSG_GROUP_CHAT,
};