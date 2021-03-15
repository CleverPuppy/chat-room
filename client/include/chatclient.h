#pragma once
#include <memory>
#include <future>
#include <list>

#include "chat.h"
#include "types.h"
#include "cmds.h"

class Client;

class RoomClient
{
public:
    RoomClient(const RoomIDType& id, const RoomNameType& name): ID(id), name(name)
    {}

    void addChatList(const Json::Value& json_lists);

    RoomIDType ID;
    RoomNameType name;
    std::list<ChatItem> chatHistory;

private:
    std::list<ChatItem>::iterator lastRead;
};

std::ostream& operator<<(std::ostream& os, RoomClient& client);

class ChatClient
{
/* TODO
 * 1. 请求所属的Room
 * 2. 请求所属Room的消息
*/
public:
    ChatClient() = default;
    
    void requestRooms(Client* clientPt);
    void requestChatInfos(Client* clientPt);            // 轮询所有Room
    void requestCurrentRoomChatInfo(Client* clientPt);  // 询问当前Room
    void runBackground(Client* clientPt);
    void printRooms(std::ostream& os);
    std::shared_ptr<RoomClient> getRoom(const RoomIDType& id);
private:
    std::map<RoomIDType, std::shared_ptr<RoomClient>> roomMap;
    void fetchMsg(Client* clientPt, uint sec, uint usec);
    std::future<void> mBackGround;

    void addRoom(const RoomIDType& id, const RoomNameType& name);
    void addRoom(const Json::Value& info);
    void addRoomAndChRoom(const Json::Value& info, Client* clientPt);

    void requestRoomChatInfo(Client* clientPt, std::shared_ptr<RoomClient>& roomClientPtr);

    UserId m_userid;    //TODO add user_id to support differentiate msg send from myself. server login module support must;
};
