#pragma once
#include <unordered_set>
#include <memory>
#include <vector>

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
        ret['id'] = ID;
        ret['name'] = name;
        ret['status'] = int(status);
        ret['creator'] = creator;
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
private:
    RoomNameType name;
    RoomIDType ID;
    ROOM_STATUS status;
    UserId creator;
    std::unordered_set<UserId> managers;
    std::unordered_set<UserId> menbers; // contains managers

    std::vector<ChatItem> mChats;
};
