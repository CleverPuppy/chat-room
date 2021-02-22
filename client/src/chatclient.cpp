#include "chatclient.h"
#include "client.h"
#include <future>

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
    uint interval_sec = 2;
    uint interval_msec = 0;
    mBackGround = std::async(&ChatClient::fetchMsg, this, clientPt, interval_sec, interval_msec);
    std::cout << "select run background\n" << std::endl;
}

void ChatClient::fetchMsg(Client* clientPt, uint seconds, uint minsec) 
{
    int clientFd = clientPt->fd;

    int nfds = clientFd + 1;
    timeval tv {seconds,minsec};
    fd_set readfds;
    int retVal;
    Json::CharReaderBuilder reader;
    Json::Value root;
    // request for room list
    requestRooms(clientPt);

    while (clientPt->status == ClientStatus::WAITING_FOR_CMD)
    {
        // request for chats
        requestChatInfos(clientPt);

        FD_ZERO(&readfds);
        FD_SET(clientFd, &readfds);
        retVal = select(nfds, &readfds, nullptr, nullptr, &tv);
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
