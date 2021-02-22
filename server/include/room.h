#pragma once
#include <unordered_set>
#include <memory>
#include <vector>
#include <algorithm>

#include "types.h"
#include "people.h"
#include "chat.h"

class People;

enum class ROOM_STATUS
{
    OPEN,
    LOCKED,
    CLOSED
};

struct RoomInfo
{
    RoomNameType name;
    RoomIDType ID;
    ROOM_STATUS status;
    UserId creator;
};

class Room
{
public:
    Room(const RoomNameType& name, 
         const RoomIDType& ID,
         const UserId& creator)
    :name(name), ID(ID), status(ROOM_STATUS::OPEN), creator(creator)
    {
        managers.insert(creator);
        menbers.insert(creator);
        // dummy head
        mChats.emplace_back(0, "dummy text");
    }
    ~Room() = default;

    void addUser(const UserId& userid)
    {
        menbers.insert(userid);
    }

    void deleteUser(const UserId& userid)
    {
        menbers.erase(userid);
    }

    void setToManager(const UserId& userid)
    {
        if(isInRoom(userid))
        {
            managers.insert(userid);
        }
    }
    
    void removeManager(const UserId& userid)
    {
        managers.erase(userid);
    }
    
    bool isInRoom(const UserId& userid) const
    {
        return bool(menbers.count(userid) == 1);
    }

    bool isManager(const UserId& userid) const
    {
        return bool(managers.count(userid) == 1);
    }

    void changeRoomName(const RoomNameType& newName)
    {
        name = newName;
    }

    bool isLocked()
    {
        return status == ROOM_STATUS::LOCKED;
    }

    bool isOpen()
    {
        return status == ROOM_STATUS::OPEN;
    }

    bool lock()
    {
        status = ROOM_STATUS::LOCKED;
    }

    bool unlock()
    {
        status = ROOM_STATUS::OPEN;
    }

    Json::Value genJsonInfo()
    {
        Json::Value ret;
        ret["id"] = ID;
        ret["name"] = name;
        ret["status"] = int(status);
        ret["creator"] = creator;
        return ret;
    }

    void addChatItem(ChatItem& item)
    {
        // ANCHOR not thread safe 
        if(item.timestamp == mChats.back().timestamp)
        {
            item.index = mChats.back().index + 1;
        }
        mChats.emplace_back(item);
    }

    void sendUnread(time_t timestamp, uint32_t index, int fd)
    {
        auto it = std::find_if(mChats.begin(), mChats.end(), [&](const ChatItem& c) -> bool{
            return bool(c.timestamp == timestamp && c.index > index) || (c.timestamp > timestamp);
        });
        if(it == mChats.end()) return;
        std::vector<ChatItem> chatsCopy{it, mChats.end()};
        auto size = chatsCopy.size();
        size_t count = 0;
        while(count + MAX_CHATS_PER_MSG < size)
        {
            auto msg = CProtoMsgManager::genInfoResponse(ResponseStatus::CHAT_LIST, Json::Value{});
            msg.body["roomid"] = ID;
            for(size_t i = 0; i < MAX_CHATS_PER_MSG; ++i)
            {
                msg.body["info"].append(chatsCopy[count].toJson());
                ++count;
            }
            CProtoMsgManager::encodeAndSendMsg(msg, fd);
        }
        if(count == size) return;
        auto msg = CProtoMsgManager::genInfoResponse(ResponseStatus::CHAT_LIST,Json::Value{});
        msg.body["roomid"] = ID;
        for(;count < size; ++count)
        {
            msg.body["info"].append(chatsCopy[count].toJson());
        }
        CProtoMsgManager::encodeAndSendMsg(msg, fd);
    }
private:
    static const int32_t MAX_CHATS_PER_MSG = 10;
    RoomNameType name;
    RoomIDType ID;
    ROOM_STATUS status;
    UserId creator;
    std::unordered_set<UserId> managers;
    std::unordered_set<UserId> menbers; // contains managers

    std::vector<ChatItem> mChats;
};
