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
        requestRoomChatInfo(clientPt, room.second);
    }
}

void ChatClient::requestCurrentRoomChatInfo(Client* clientPt) 
{
    if(clientPt->isRoomSelected()){
        requestRoomChatInfo(clientPt, clientPt->currentRoomClient);
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
        // requestChatInfos(clientPt);
        requestCurrentRoomChatInfo(clientPt);
        timeout.tv_sec = sec;
        timeout.tv_usec = usec;
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(clientFd, &readfds);
        retVal = select(nfds, &readfds, nullptr, nullptr, &timeout);
        std::vector<int> closedFds;
        switch (retVal)
        {
        case -1:
            fprintf(stderr, "select error\n");
            break;
        case 1:
            clientPt->msgManager.readData(clientFd, closedFds);
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
                    if(clientPt->isRoomSelected()){
                        addRoom(info);
                    }else{
                        addRoomAndChRoom(info, clientPt);
                    }
                    break;
                case ResponseStatus::ROOM_JOIN_SUCCESS:
                    fprintf(stdout, "Room joined\n");
                    if(clientPt->isRoomSelected()){
                        addRoom(info);
                    }else{
                        addRoomAndChRoom(info, clientPt);
                    }
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
                    if(info.size() == 1){
                        addRoomAndChRoom(info[0], clientPt);
                    }else{
                        for(Json::ArrayIndex i = 0; i < info.size(); ++i)
                        {
                            addRoom(info[i]);
                        }
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
        if(!closedFds.empty())
        {
            for(int fd: closedFds)
            {
                clientPt->msgManager.releaseConnection(fd);
            }
            clientPt->status = ClientStatus::FAILED_CONNECT_TO_SERVER;
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

void ChatClient::addRoomAndChRoom(const Json::Value& info, Client* clientPt) 
{
    RoomIDType roomid = info["id"].asUInt();
    RoomNameType room_name = info["name"].asString();
    if(roomid)
    {
        addRoom(roomid, room_name);
    }
    auto room = getRoom(roomid);
    clientPt->chRoom(room);
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

std::ostream& operator<<(std::ostream& os, RoomClient& client) 
{
    os << client.name << "-" << client.ID;
    return os;
}

void ChatClient::printRooms(std::ostream& os) 
{
    os << "Rooms:"<<std::endl;
    if(roomMap.empty()){
        os << "None." << std::endl;
    }
    int index = 1;
    for(const auto& p : roomMap)
    {
        os <<index++ << " " << *p.second << std::endl;
    }
}

void ChatClient::requestRoomChatInfo(Client* clientPt, std::shared_ptr<RoomClient>& roomClientPtr) 
{
        auto& roomid = roomClientPtr->ID;
        time_t lastTime = 0;
        uint32_t index = 0;

        if(!roomClientPtr->chatHistory.empty())
        {
            const auto& chatitem = roomClientPtr->chatHistory.back();
            lastTime = chatitem.timestamp;
            index = chatitem.index;
        }
        
        auto requestMsg = clientPt->msgManager.genTokenCmdRequest(CMD_CHAT_GET, clientPt->token, roomid, lastTime, index);
        clientPt->msgManager.encodeAndSendMsg(requestMsg, clientPt->fd);
}
