#include "chatclient.h"
#include "client.h"
#include <future>



void ChatClient::requestRooms(Client* clientPt) 
{
    auto msg = clientPt->msgManager.genCmdRequest(CMD_ROOM_LIST, clientPt->token);
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
        
        auto requestMsg = clientPt->msgManager.genCmdRequest(CMD_CHAT_GET, roomid, lastTime, index);
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

void ChatClient::fetchMsg(Client* clientPt, uint seconds, uint minsec) 
{
    int clientFd = clientPt->fd;

    int nfds = clientFd + 1;
    timeval tv {seconds,minsec};
    fd_set readfds;
    int retVal;


    while (clientPt->status == ClientStatus::WAITING_FOR_CMD)
    {
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
            }
            break;
        default:
            break;
        }
    }
}
