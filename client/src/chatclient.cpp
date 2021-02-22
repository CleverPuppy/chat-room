#include "chatclient.h"
#include "client.h"
#include <future>
#include <algorithm>

void ChatClient::requestRooms(Client* clientPt) 
{
    auto msg = clientPt->msgManager.genTokenCmdRequest(CMD_ROOM_LIST, clientPt->token);
    clientPt->msgManager.encodeAndSendMsg(msg, clientPt->fd);
}

void ChatClient::requestChatInfos(Client* clientPt) 
{
    for(auto& room : roomMap)
    {
        auto& roomid = room.first;
        time_t lastTime = 0;
        uint32_t index = 0;

        if(!room.second->chatHistory.empty())
        {
            const auto& chatitem = room.second->chatHistory.back();
            lastTime = chatitem.timestamp;
            index = chatitem.index;
        }
        
        auto requestMsg = clientPt->msgManager.genTokenCmdRequest(CMD_CHAT_GET, clientPt->token, roomid, lastTime, index);
        clientPt->msgManager.encodeAndSendMsg(requestMsg, clientPt->fd);
    }
}

void ChatClient::runBackground(Client* clientPt) 
{
    uint interval_sec = 1;
    uint interval_msec = 0;
    mBackGround = std::async(&ChatClient::fetchMsg, this, clientPt, interval_sec, interval_msec);
    std::cout << "select run background\n" << std::endl;
}

void ChatClient::fetchMsg(Client* clientPt, uint sec, uint usec) 
{
    int clientFd = clientPt->fd;
    timeval timeout{sec, usec};
    int nfds = clientFd + 1;
    int retVal;
    Json::CharReaderBuilder reader;
    Json::Value root;
    // request for room list
    requestRooms(clientPt);

    while (clientPt->status == ClientStatus::WAITING_FOR_CMD)
    {
        // request for chats
        requestChatInfos(clientPt);
        timeout.tv_sec = sec;
        timeout.tv_usec = usec;
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(clientFd, &readfds);
        retVal = select(nfds, &readfds, nullptr, nullptr, &timeout);
        switch (retVal)
        {
        case -1:
            fprintf(stderr, "select error\n");
            break;
        case 1:
            clientPt->msgManager.readData(clientFd);
            while (auto msgPt = clientPt->msgManager.getMsg(clientFd))
            {
                std::cout << msgPt->body << std::endl;
                ResponseStatus status = CProtoMsgManager::getMsgStatus(*msgPt);
                auto& info = msgPt->body["info"];
                RoomIDType roomid;
                switch (status)
                {
                case ResponseStatus::INTERNAL_ERROR:
                    fprintf(stderr, "INTERNAL ERROR\n");
                    break;
                case ResponseStatus::ROOM_CREATE_SUCCESS:
                    fprintf(stdout, "Room created\n");
                    addRoom(info);
                    break;
                case ResponseStatus::CHAT_LIST:
                    roomid = msgPt->body["roomid"].asUInt();
                    if(getRoom(roomid))
                    {
                        getRoom(roomid)->addChatList(info);
                    }else
                    {
                        fprintf(stderr, "wrong roomid\n");
                    }
                    break;
                case ResponseStatus::ROOM_LIST:
                    for(Json::ArrayIndex i = 0; i < info.size(); ++i)
                    {
                        addRoom(info[i]);
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        case 0:
            break;
        default:
            break;
        }
    }
}

void ChatClient::addRoom(const RoomIDType& id, const RoomNameType& name) 
{
    if(roomMap.count(id) != 0)
    {
        roomMap[id]->name = name;
        return;
    }

    auto room = std::shared_ptr<RoomClient>(new RoomClient{id, name});
    roomMap[id] = room;
}

void ChatClient::addRoom(const Json::Value& info) 
{
    RoomIDType roomid = info["id"].asUInt();
    RoomNameType room_name = info["name"].asString();
    if(roomid)
    {
        addRoom(roomid, room_name);
    }
}

std::shared_ptr<RoomClient> ChatClient::getRoom(const RoomIDType& id) 
{
    return roomMap[id];
}

void RoomClient::addChatList(const Json::Value& json_lists) 
{
    std::list<ChatItem> chats;
    for(Json::ArrayIndex i = 0; i < json_lists.size(); ++i)
    {
        chats.emplace_back(json_lists[i]);
    }
    chatHistory.merge(chats);
    printf("Msgs in room [%u][%lu]s\n", ID, chatHistory.size());
}
