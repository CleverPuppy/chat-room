#include "chat.h"
#include "cmds.h"

ChatItem::ChatItem(const UserId& sender, const std::string& text) 
    :sender(sender), text(text) ,timestamp(time(nullptr))
{
    
}

ChatItem::ChatItem(const Json::Value& jsonChatItem)
    :sender(jsonChatItem["sender"].asInt()), 
    text(jsonChatItem["text"].asString()), 
    timestamp(jsonChatItem["timestamp"].asInt())
{

}

std::ostream& operator<<(std::ostream& os, const ChatItem& chatItem) 
{
    os << chatItem.sender << " : " << chatItem.text;
}

Json::Value ChatItem::toJson() 
{
    Json::Value ret;
    ret["timestamp"] = timestamp;
    ret["sender"] = sender;
    ret["text"] = text;
    return ret;
}

