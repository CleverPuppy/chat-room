#include "cprotomsg.h"
#include <iostream>
#include "unistd.h"
#include "sys/socket.h"
#include "cmds.h"


void CProtoMsgManager::establishNewConnection(int fd) 
{
    if(fdMap[fd] == nullptr)
    {
        fdMap[fd].reset(new CProtoDecoder{});
    }else
    {
        fprintf(stderr, "CProtoMsgManager::establishNewConnection, fd already used!\n");
    }
}


void CProtoMsgManager::releaseConnection(int fd) 
{
    if (fdMap[fd] != nullptr)
    {
        if(close(fd) == -1)
        {
            fprintf(stderr, "close fd : %d failed\n", fd);
        };
    }
    fdMap[fd] = nullptr;
}

void CProtoMsgManager::readData(int fd) 
{
    auto decoder = fdMap[fd];
    if(decoder == nullptr)
    {
        fprintf(stderr,"fd %d don't have valid decoder\n", fd);
        return;
    }
    char buffer[2048];
    int read_num = 0;
    while (( read_num = read(fd, buffer, sizeof(buffer))) > 0)
    {
        decoder->parser(buffer, read_num);
    }
    if(read_num == -1 && errno != EAGAIN)
    {
        perror("read error\n");
    }
}

void CProtoMsgManager::readData(int fd, std::vector<int>& closed) 
{
    auto decoder = fdMap[fd];
    if(decoder == nullptr)
    {
        fprintf(stderr,"fd %d don't have valid decoder\n", fd);
        return;
    }
    char buffer[2048];
    int read_num = 0;
    bool shouldClose = true;
    while (( read_num = read(fd, buffer, sizeof(buffer))) > 0)
    {
        shouldClose = false;
        decoder->parser(buffer, read_num);
    }
    if(shouldClose){
        closed.push_back(fd);
    }
    if(read_num == -1 && errno != EAGAIN)
    {
        perror("read error\n");
    }
}

std::shared_ptr<CProtoMsg> CProtoMsgManager::getMsg(int fd) 
{
    auto decoder = fdMap[fd];
    if(decoder == nullptr)
    {
        fprintf(stderr,"fd %d don't have valid decoder\n", fd);
        return {};
    }
    if(decoder->empty())
    {
        return {};
    }
    auto&& msg = decoder->front();
    decoder->pop();
    return msg;
}

int CProtoMsgManager::encodeAndSendMsg(CProtoMsg& msg, int fd) 
{
    int ret;
    uint32_t len;
    auto dataPtr = CProtoEncoder::encode(&msg, len);
    if((ret = send(fd, dataPtr.get(), len, MSG_CONFIRM) )== -1)
    {
        fprintf(stderr, "send ERROR :%s\n", strerror(errno));
    };
    // std::cout << "sendMsg : " << msg.body << std::endl;
    return ret;
}

CProtoMsg CProtoMsgManager::genChatMsg(const UserToken& token,
                                const RoomIDType& roomID,
                                const std::string& info) 
{
    auto msg = genTokenCmdRequest(CMD_CHAT_SEND, token, roomID, info);
    return msg;
}
