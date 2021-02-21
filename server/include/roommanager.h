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

    RoomIDType addRoom(const RoomNameType& name, 
                 const UserId& creator);
    void joinRoom(const RoomIDType& roomId, const UserId& userId);
    RoomPtType getRoomPt(const RoomIDType& id) ;
    bool isRoomExist(const RoomIDType& id) const;

    bool verifyRoomExist(const RoomIDType& roomid, int fd);
    bool verifyRoomOpen(const RoomIDType& roomid, int fd);
    bool verifyUserInRoom(const RoomIDType& roomid, const UserId& userid, int fd);
    bool verifyUserIsManager(const RoomIDType& roomid, const UserId& userid, int fd);

    CProtoMsg genRoomListResponse(const std::set<RoomIDType>& rooms);
private:
    std::unordered_map<RoomIDType, RoomPtType> idMap;
    RoomIDType currentMaxRoomID = 10001;
    RoomIDType roomIDGenerator();
};