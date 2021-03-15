#pragma once
#include <ctime>
#include <iostream>
#include <unordered_map>
#include <map>
#include "types.h"
#include "cprotomsg.h"
#include "json/json.h"

class ChatItem
{
public:
    ChatItem(const UserId& sender, const UserName& name, const std::string& text);
    ChatItem(const ChatItem& c);
    ChatItem(const Json::Value& jsonChatItem);
    Json::Value toJson();

    time_t timestamp;
    uint32_t index;     // same timestamp index
    UserId sender;
    UserName sender_name;
    std::string text;

};
bool operator<(const ChatItem& lhs, const ChatItem& rhs);
