#pragma once
#include "room.h"
#include "types.h"
#include <unordered_map>
#include <memory>

using RoomPtType = std::shared_ptr<Room>;

class RoomManager
{
public:
    RoomManager() = default;
    RoomManager(const RoomManager&) = delete;
    RoomManager& operator=(const RoomManager&) = delete;

    void addRoom(const RoomNameType& name, 
                 const UserId& creator);
    RoomPtType getRoomPt(const RoomIDType& id);
private:
    std::unordered_map<RoomIDType, RoomPtType> idMap;
    RoomIDType currentMaxRoomID = 10001;
    RoomIDType roomIDGenerator();
    bool isRoomExist(const RoomIDType id) const;
};