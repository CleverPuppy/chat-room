#include "chat.h"
#include "cmds.h"

ChatItem::ChatItem(const UserId& sender, const UserName& name, const std::string& text) 
    :sender(sender),sender_name(name), text(text) ,timestamp(time(nullptr)), index(0)
{
    
}

ChatItem::ChatItem(const Json::Value& jsonChatItem)
    :sender(jsonChatItem["sender"].asInt()), 
    text(jsonChatItem["text"].asString()), 
    timestamp(jsonChatItem["timestamp"].asInt()),
    sender_name(jsonChatItem["sender_name"].asString())
{

}

ChatItem::ChatItem(const ChatItem& c) 
    :sender(c.sender), text(c.text), timestamp(c.timestamp), index(c.index), sender_name(c.sender_name)
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
    ret["index"] = index;
    ret["sender_name"] = sender_name;
    return ret;
}


bool operator<(const ChatItem& lhs, const ChatItem& rhs) 
{
    return (lhs.timestamp < rhs.timestamp || lhs.timestamp == rhs.timestamp && lhs.index < rhs.index);

}
