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
    ChatItem(const UserId& sender, const std::string& text);
    ChatItem(const Json::Value& jsonChatItem);
    Json::Value toJson();

    time_t timestamp;
    uint32_t index;     // same timestamp index
    UserId sender;
    std::string text;
};
