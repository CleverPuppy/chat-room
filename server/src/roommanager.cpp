#include "roommanager.h"
#include "cprotomsg.h"


RoomIDType RoomManager::roomIDGenerator() 
{
    return ++ currentMaxRoomID;
}

RoomIDType RoomManager::addRoom(const RoomNameType& name, 
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
    return roomID;
}

bool RoomManager::isRoomExist(const RoomIDType& id) const
{
    return bool(idMap.count(id) == 1);
}

RoomPtType RoomManager::getRoomPt(const RoomIDType& id)
{
    if (isRoomExist(id))
    {
        return idMap.at(id);
    }
    return {};
}


void RoomManager::joinRoom(const RoomIDType& roomId, const UserId& userId) 
{
    auto& roomPt = idMap.at(roomId);
    roomPt->addUser(userId);
}

bool RoomManager::verifyRoomExist(const RoomIDType& roomid, int fd) 
{
    if(isRoomExist(roomid))
    {
        return true;
    }

    // room not exist
    std::string info = "Invalid RoomId";
    auto msg = CProtoMsgManager::genInfoResponse(ResponseStatus::INTERNAL_ERROR, info);
    CProtoMsgManager::encodeAndSendMsg(msg, fd);
    return false;
}

bool RoomManager::verifyRoomOpen(const RoomIDType& roomid, int fd) 
{
    auto roomPt = getRoomPt(roomid);
    if(roomPt->isOpen())
    {
        return true;
    }
    // room not open
    std::string info = "Room not open";
    auto msg = CProtoMsgManager::genInfoResponse(ResponseStatus::INTERNAL_ERROR, info);
    CProtoMsgManager::encodeAndSendMsg(msg, fd);
    return false;
}

CProtoMsg RoomManager::genRoomListResponse(const std::set<RoomIDType>& rooms)
{
    auto msg = CProtoMsgManager::genInfoResponse(ResponseStatus::ROOM_LIST, "");
    for(auto roomid : rooms)
    {
        auto roomPt = getRoomPt(roomid);
        auto roomInfo = roomPt->genJsonInfo();
        msg.body["roomlist"].append(roomInfo);
    }
    return msg;
}

bool RoomManager::verifyUserIsManager(const RoomIDType& roomid, const UserId& userid, int fd) 
{
    auto room = getRoomPt(roomid);
    if(room->isManager(userid)){
        return true;
    }

    auto msg = CProtoMsgManager::genInfoResponse(ResponseStatus::INTERNAL_ERROR, "DO NOT HAVE PERMISSION");
    CProtoMsgManager::encodeAndSendMsg(msg, fd);
    return false;
}

bool RoomManager::verifyUserInRoom(const RoomIDType& roomid, const UserId& userid, int fd) 
{
    auto room = getRoomPt(roomid);
    if(room->isInRoom(userid)){
        return true;
    }

    auto msg = CProtoMsgManager::genInfoResponse(ResponseStatus::INTERNAL_ERROR, "DO NOT HAVE PERMISSION");
    CProtoMsgManager::encodeAndSendMsg(msg, fd);
    return false;
}
