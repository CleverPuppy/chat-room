#include "roommanager.h"


RoomIDType RoomManager::roomIDGenerator() 
{
    return ++ currentMaxRoomID;
}

void RoomManager::addRoom(const RoomNameType& name, 
                          const UserId& creator) 
{
    auto roomID = roomIDGenerator();
    RoomPtType roomPt;
    roomPt.reset(new Room{name, roomID, creator});
    if(isRoomExist(roomID))
    {
        fprintf(stderr, "Wrong ROOMID\n Fatal error\n");
        std::abort();
    }
    idMap.insert({roomID, roomPt});
}

bool RoomManager::isRoomExist(const RoomIDType id) const
{
    return bool(idMap.count(id) == 1);
}

RoomPtType RoomManager::getRoomPt(const RoomIDType& id) 
{
    if (isRoomExist(id))
    {
        return idMap[id];
    }
    return {};
}
