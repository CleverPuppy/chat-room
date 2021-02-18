#pragma once
#include <unordered_set>
#include <memory>

#include "types.h"
#include "people.h"

class People;

enum class ROOM_STATUS
{
    OPEN,
    LOCKED,
    CLOSED
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
private:
    RoomNameType name;
    RoomIDType ID;
    ROOM_STATUS status;
    UserId creator;
    std::unordered_set<UserId> managers;
    std::unordered_set<UserId> menbers; // contains managers
};
