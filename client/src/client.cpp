#include "client.h"
#include "netutils.h"
#include "login.h"
#include "cmds.h"
#include <cstdio>
#include <string>
#include <iostream>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <cassert>
#include <thread>
#include "unistd.h"
#include "sys/time.h"
#include "sys/types.h"

constexpr const char* SERVER_ADDRESS = "127.0.0.1";
constexpr uint16_t PORT = 8080;

Client::Client() 
    :status(ClientStatus::START), fd(-1)
{
    connectToServer();
}

Client::~Client() 
{
    if(fd > 0)
    {
        close(fd);
    }
}

void Client::connectToServer() 
{
    if(status != ClientStatus::START && status != ClientStatus::FAILED_CONNECT_TO_SERVER)
    {
        return;
    }

    printf("connecting to server %s@%d...\n",SERVER_ADDRESS, PORT);
    fd = NetUtils::create_socket();
    if (fd <= 0)
    {
        NetUtils::handle_error("Counld't create_socket!\n");
    }
    sockaddr addr;
    NetUtils::set_sockaddr(SERVER_ADDRESS, PORT, &addr);
    int conFlag = connect(fd, &addr, sizeof(addr));
    if(conFlag == 0)
    {
        printf("Connected!\n");
        msgManager.establishNewConnection(fd);
        status = ClientStatus::WAITING_FOR_LOGING;
    }else if(conFlag == -1)
    {
        printf("Connect To Server Failed!\n%s\n", strerror(errno));
        status = ClientStatus::FAILED_CONNECT_TO_SERVER;
    }
    NetUtils::make_socket_unblock(fd);
}

void Client::waitingForLogin() 
{
    while (status == ClientStatus::WAITING_FOR_LOGING)
    {
        if(isUserTokenValid()){
            printf("Using UserToken\n");
            status = ClientStatus::WAITING_FOR_CMD;
            break;
        }
        waitingForLoginHint();
        std::string line;
        std::vector<std::string> cmds;
        if (std::getline(std::cin, line) && status == ClientStatus::WAITING_FOR_LOGING)
        {
            cmds.clear();
            std::stringstream ss{line};
            std::string tmp;
            while(ss >> tmp)
            {
                cmds.push_back(tmp);
            }
            if (isCmdValid(cmds))
            {
                if(cmds.front() == LOGIN_CMD)
                {
                    Login::login(cmds[1], cmds[2], fd);
                }
                if(cmds.front() == REGISTER_CMD)
                {
                    Login::registerAndLogin(cmds[1],cmds[2], fd);
                }

                fd_set rfds;
                timeval timeout{5, 0};
                FD_ZERO(&rfds);
                FD_SET(fd, &rfds);
                int select_ret = select(fd + 1, &rfds, NULL, NULL, &timeout);
                std::vector<int> closedFds;
                if(select_ret == -1)
                {
                    fprintf(stderr, "select() error\n");
                }
                else if(select_ret)
                {
                    msgManager.readData(fd, closedFds);
                    if(auto msgPt = msgManager.getMsg(fd))
                    {
                        // std::cout << msgPt->body << std::endl;
                        if(CProtoMsgManager::isMsgSuccess(*msgPt))
                        {
                            token = msgPt->body["info"].asString();
                            printf("Login Successful!\n");
                            status = ClientStatus::WAITING_FOR_CMD;
                        }else{
                            auto info = msgPt->body["info"].asString();
                            printf("Login Failed, error %s\n", info.c_str());
                        }
                    }
                }else{
                    fprintf(stderr, "timeout\n");
                }
                if(!closedFds.empty())
                {
                    for(int fd: closedFds)
                    {
                        msgManager.releaseConnection(fd);
                    }
                    status = ClientStatus::FAILED_CONNECT_TO_SERVER;
                }
            }else break;
        }
    }
}

void Client::waitingForLoginHint() 
{
    printf("\t\t\t\t\tLogin or Register\n\n\n\n"
            "Cmds:\n"
            "Login:   \t\tlogin [username] [password]\n"
            "Register:\t\tregister [username] [password]\n");
}

bool Client::isCmdValid(const std::vector<std::string>& cmds) 
{
    if(cmds.empty()) return false;
    switch (status)
    {
    case ClientStatus::WAITING_FOR_LOGING:
        if(cmds.front() != CMD_REGISTER && cmds.front() != CMD_LOGIN)
        {
            wrongCmdNameHint(cmds.front());
            return false;
        }
        if(cmds.size() != 3)
        {
            wrongCmdSizeHint(cmds.front(), cmds.size() - 1, 3 - 1);
            return false;
        }
        break;
    case ClientStatus::WAITING_FOR_CMD:
        if(cmds.front() == CMD_ROOM_LOCK   ||
                 cmds.front() == CMD_ROOM_UNLOCK ||
                 cmds.front() == CMD_ROOM_JOIN   ||
                 cmds.front() == CMD_ROOM_CREATE ||
                 cmds.front() == CMD_ROOM_CHANGE
            )
        {
            if(cmds.size() != 2)
            {
                wrongCmdSizeHint(cmds.front(), cmds.size() - 1, 2 - 1);
                return false;
            }
        }else if(cmds.front() == CMD_CHAT_SEND)
        {
            if(!isRoomSelected())
            {
                wrongRoomNotSelectedHint();
                return false;
            }
            if(cmds.size() != 2)
            {
                wrongCmdSizeHint(cmds.front(), cmds.size() - 1, 2 - 1);
                return false;
            }
        }else if(cmds.front() == CMD_QUIT ||
                 cmds.front() == CMD_ROOM_LIST)
        {
            if(cmds.size() != 1)
            {
                wrongCmdSizeHint(cmds.front(), cmds.size() - 1, 1 - 1);
                return false;
            }
        }
        else{
            wrongCmdNameHint(cmds.front());
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}


void Client::wrongCmdSizeHint(const std::string& cmd, int real, int target) 
{
    fprintf(stderr, "comman [%s] must have %d argv[s] given, but %d was given\n", 
            cmd.c_str(), target, real);
}

void Client::wrongCmdNameHint(const std::string& cmd) 
{
    fprintf(stderr, "invalid cmd [%s]\n", cmd.c_str());
}

void Client::mainLoop() 
{
    printf("Main loop\n");
    while(true)
    {
        statusTransfrom();
    }
}

void Client::statusTransfrom() 
{
    switch (status)
    {
    case ClientStatus::FAILED_CONNECT_TO_SERVER:
        if(++retryTimes > MAX_RETRY_TIMES)
        {
            fprintf(stderr, "Reached max retry times still failed to connect to server\n");
            status = ClientStatus::EXIT;
        }else
        {
            constexpr uint32_t waitingSec = 3;
            printf("Waiting %d seconds...\n", waitingSec);
            sleep(waitingSec);
            printf("Retry ...%d\n", retryTimes);
            connectToServer();
        }
        break;
    case ClientStatus::WAITING_FOR_LOGING:
        retryTimes = 0;
        waitingForLogin();
        break;
    case ClientStatus::WAITING_FOR_CMD:
        waitingForCmd();
        break;
    case ClientStatus::EXIT:
        exit(EXIT_SUCCESS);
        break;
    default:
        exit(EXIT_FAILURE);
        break;
    }
}

void Client::waitingForCmdHint() 
{
    fprintf(stdout, 
            "Cmds:\n"
            "Create Room:   \t\t createroom [roomname]\n"
            "Join Room:     \t\t joinroom [roomid]\n"
            "Lock Room:     \t\t lockroom [roomid]\n"
            "Unlock Room:   \t\t unlockroom [roomid]\n"
            "Change Room:   \t\t chroom [roomid]\n"
            "List Rooms:    \t\t lsroom\n"
            "Quit:          \t\t quit\n");
    roomCmdHint();
    fflush(stdout);
}

void Client::waitingForCmd() 
{
    waitingForCmdHint();
    chatClient.runBackground(this);
    while (status == ClientStatus::WAITING_FOR_CMD)
    {
        std::string line;
        std::vector<std::string> cmds;
        if (std::getline(std::cin, line) && status == ClientStatus::WAITING_FOR_CMD)
        {
            cmds.clear();
            std::stringstream ss{line};
            std::string tmp;
            while(ss >> tmp)
            {
                cmds.push_back(tmp);
            }
            if(isCmdValid(cmds))
            {
                if(cmds.front() == CMD_ROOM_CREATE)
                {
                    auto msg = CProtoMsgManager::genTokenCmdRequest(CMD_ROOM_CREATE, token, cmds[1]);
                    msgManager.encodeAndSendMsg(msg, fd);
                }else if(cmds.front() == CMD_ROOM_JOIN)
                {
                    RoomIDType roomId = std::stoi(cmds[1]);
                    auto msg = CProtoMsgManager::genTokenCmdRequest(CMD_ROOM_JOIN, token, roomId);
                    msgManager.encodeAndSendMsg(msg, fd);
                }else if(cmds.front() == CMD_ROOM_UNLOCK)
                {
                    RoomIDType roomId = std::stoi(cmds[1]);
                    auto msg = CProtoMsgManager::genTokenCmdRequest(CMD_ROOM_UNLOCK, token, roomId);
                    msgManager.encodeAndSendMsg(msg, fd);
                }else if(cmds.front() == CMD_ROOM_LOCK)
                {
                    RoomIDType roomId = std::stoi(cmds[1]);
                    auto msg = CProtoMsgManager::genTokenCmdRequest(CMD_ROOM_LOCK, token, roomId);
                    msgManager.encodeAndSendMsg(msg, fd);
                }else if(cmds.front() == CMD_CHAT_SEND)
                {
                    if(isRoomSelected()){
                        RoomIDType roomId = currentRoomClient->ID;
                        auto msg = CProtoMsgManager::genChatMsg(token, roomId, cmds[1]);
                        msgManager.encodeAndSendMsg(msg, fd);
                    }else{
                        wrongRoomNotSelectedHint();
                    }
                }else if(cmds.front() == CMD_ROOM_LIST)
                {
                    chatClient.printRooms(std::cout);
                }else if(cmds.front() == CMD_ROOM_CHANGE)
                {
                    RoomIDType roomId = std::stoi(cmds[1]);
                    auto roomPtr = chatClient.getRoom(roomId);
                    if(roomPtr){
                        chRoom(roomPtr);
                    }else{
                        fprintf(stderr, "Wrong roomid, please join room[%d] first\n", roomId);
                    }
                }else if(cmds.front() == CMD_QUIT)
                {
                    status = ClientStatus::EXIT;
                }else{
                    fprintf(stderr, "unknow cmd\n");
                }
            }
        }
    }
}

bool Client::isUserTokenValid() 
{
    return !token.empty();
}

bool Client::isRoomSelected() 
{
    return currentRoomClient != nullptr;
}

void Client::roomCmdHint() 
{
    if(isRoomSelected()){
        fprintf(stdout,
                "Cmds for Room [%s]-%d\n"
                "SendMessage：  \t\t send [info]\n"
                "change Room:   \t\t chroom [roomid]\n", 
                currentRoomClient->name.c_str(), currentRoomClient->ID);
    }
}

void Client::wrongRoomNotSelectedHint() 
{
    fprintf(stderr, "Room Not Selected, use cmd : chroom [roomid]\n");
}


void Client::chRoom(std::shared_ptr<RoomClient>& ptr) 
{
    if(ptr){
        currentRoomClient = ptr;
        roomChangedHint();
    }
}

void Client::roomChangedHint() 
{   
    fprintf(stdout, "Current in Room [%s]-%d\n", currentRoomClient->name.c_str(),
                                            currentRoomClient->ID);
    roomCmdHint();
}
