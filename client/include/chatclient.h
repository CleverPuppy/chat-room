#pragma once
#include <memory>
#include <future>

#include "chat.h"
#include "types.h"
#include "cmds.h"

class Client;

class RoomClient
{
public:
    RoomClient(RoomIDType id): ID(id) 
    {}

    RoomIDType ID;
    RoomNameType name;
    std::vector<ChatItem> chatHistory;
};

class ChatClient
{
/* TODO
 * 1. 请求所属的Room
 * 2. 请求所属Room的消息
*/
public:
    ChatClient() = default;
    
    void requestRooms(Client* clientPt);
    void requestChatInfos(Client* clientPt);

    void runBackground(Client* clientPt);
private:
    std::map<RoomIDType, std::shared_ptr<RoomClient>> roomMap;
    void fetchMsg(Client* clientPt, uint seconds, uint minsec);
    std::future<void> mBackGround;
};

std::ostream& operator<<(std::ostream& os, const ChatItem& chatItem);