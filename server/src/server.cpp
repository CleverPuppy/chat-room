#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "sys/epoll.h"
#include "stdio.h"
#include "unistd.h"

#include "server.h"
#include "netutils.h"
#include "login.h"
#include "cmds.h"
#include "chat.h"
#include <iostream>

constexpr const char* SERVER_ADDRESS = "127.0.0.1";
constexpr uint16_t PORT = 8080;

Server::Server()
{
    listen_sock = NetUtils::create_socket();
    if (listen_sock != -1)
    {
        NetUtils::make_socket_unblock(listen_sock);
        NetUtils::bind_and_listen(listen_sock, SERVER_ADDRESS, PORT);
    }
    epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        NetUtils::handle_error("epoll_create1");
    }

    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1)
    {
        NetUtils::handle_error("epoll_ctl: listen_sock");
    }
    printf("Server Starting %s@%d\n", SERVER_ADDRESS, PORT);
}

Server::~Server()
{
    close(listen_sock);
    close(epollfd);
}

void Server::run() 
{
    int ndfs, conn_sock;
    sockaddr peer_addr;
    socklen_t peer_addr_len;
    for (;;)
    {
        ndfs = epoll_wait(epollfd, events, MAX_EVENTS, -1); // -1 as infinite
        if (ndfs == -1)
        {
            NetUtils::handle_error("epoll_wait");
        }

        for (int n = 0; n < ndfs; ++n)
        {
            if(events[n].data.fd == listen_sock)
            {
                conn_sock = accept(listen_sock, &peer_addr, &peer_addr_len);
                if(conn_sock == -1)
                {
                    NetUtils::handle_error("accept");
                }
                NetUtils::make_socket_unblock(conn_sock);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;
                if(epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1)
                {
                    NetUtils::handle_error("epoll_ctl");
                }
                msgManager.establishNewConnection(conn_sock);
                #ifndef NDEBUG
                printf("New connecting\n");
                #endif
            }else
            {
                int incommingFd = events[n].data.fd;
                #ifndef NDEBUG
                printf("Reading from %d\n", incommingFd);
                #endif
                msgManager.readData(incommingFd);
                while (auto msgPt = msgManager.getMsg(incommingFd))
                {
                    std::cout << msgPt->body << std::endl;
                    loginHandler(msgPt, incommingFd);
                    roomHandle(msgPt, incommingFd);
                    chatHandler(msgPt, incommingFd);
                }
            }
        }
    }
}

void Server::loginHandler(const MsgPtType& msgPt, int fd) 
{
    if(msgPt->body["cmd"] == LOGIN_CMD || msgPt->body["cmd"] == REGISTER_CMD)
    {
        auto args = msgPt->body["args"];
        if(args.isArray())
        {
            if(args.size() != 2)
            {
                fprintf(stderr, "wrong args size\n");
                return;
            }
            std::string username = args[0].asString();
            std::string password = args[1].asString();
            #ifndef NDEBUG
            fprintf(stdout, "username : %s, password: %s\n", username.c_str(), password.c_str());
            #endif
            if(msgPt->body["cmd"] == LOGIN_CMD)
            {
                userManager.loginUser(fd, username, password);
            }
            if(msgPt->body["cmd"] == REGISTER_CMD)
            {
                userManager.registerAndLoginUser(fd, username, password);
            }
        }else
        {
            fprintf(stderr, "invalid request\n");
        }
    }
}

void Server::roomHandle(const MsgPtType& msgPt, int fd) 
{
    auto& msgBody = msgPt->body;
    auto cmd_name = msgBody["cmd"];
    if(cmd_name == CMD_ROOM_CREATE)
    {
        const auto& roomName = msgBody["args"][0].asString();
        const auto& creatorId = userManager.getUserIdFromToken(msgManager.getToken(*msgPt));
        std::string info;
        ResponseStatus status = ResponseStatus::ROOM_CREATE_FAILED;
        if(creatorId)
        {
            auto roomId = roomManager.addRoom(roomName, creatorId);
            if(roomId){
                userManager.getUser(creatorId)->joinRoom(roomId);
                status = ResponseStatus::ROOM_CREATE_SUCCESS;
                assert(roomManager.getRoomPt(roomId) != nullptr);
                auto roominfo = roomManager.getRoomPt(roomId)->genJsonInfo();
                assert(roominfo != Json::nullValue);
                auto msg = CProtoMsgManager::genInfoResponse(status, roominfo);
                msgManager.encodeAndSendMsg(msg, fd);
                return;
            }
        }
        auto msg = msgManager.genInfoResponse(status, info);
        msgManager.encodeAndSendMsg(msg, fd);
        return;
    }
    if(cmd_name == CMD_ROOM_JOIN)
    {
        auto userId = userManager.verifyTokenAndGetUserId(*msgPt, fd);
        if(userId)
        {
            RoomIDType roomId = 0;
            try
            {
                roomId = msgPt->body["args"].get(0U, 0U ).asUInt();
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
            
            if(roomManager.verifyRoomExist(roomId, fd) && roomManager.verifyRoomOpen(roomId, fd))
            {
                roomManager.joinRoom(roomId, userId);
                auto userPt = userManager.getUser(userId);
                userPt->joinRoom(roomId);
            }
        }
        return;
    }
    if(cmd_name == CMD_ROOM_LIST)
    {
        auto userId = userManager.verifyTokenAndGetUserId(*msgPt, fd);
        if(userId)
        {
            auto userPt = userManager.getUser(userId);
            auto msg = roomManager.genRoomListResponse(userPt->rooms);
            CProtoMsgManager::encodeAndSendMsg(msg, fd);
        }
        return;
    }
    if(cmd_name == CMD_ROOM_LOCK || cmd_name == CMD_ROOM_UNLOCK)
    {
        auto userId = userManager.verifyTokenAndGetUserId(*msgPt, fd);
        RoomIDType roomId = msgPt->body["args"].get(0U, 0 ).asUInt();
        if(userId && 
            roomManager.verifyRoomExist(roomId, fd) && 
            roomManager.verifyUserIsManager(roomId, userId, fd))
        {
            if(cmd_name == CMD_ROOM_LOCK)
                roomManager.getRoomPt(roomId)->lock();
            else
                roomManager.getRoomPt(roomId)->unlock();

            auto msg = CProtoMsgManager::genInfoResponse(ResponseStatus::SUCCESS, 
                        std::to_string(roomId) + ((cmd_name == CMD_ROOM_LOCK)?":roomlocked":":roomunlocked"));
            CProtoMsgManager::encodeAndSendMsg(msg, fd);
        }
        return;
    }
}

void Server::chatHandler(const MsgPtType& msgPt, int fd) 
{
    auto& msgBody = msgPt->body;
    auto cmd_name = msgBody["cmd"];
    if(cmd_name != CMD_CHAT_SEND && cmd_name != CMD_CHAT_GET)
    {
        return;
    }
    auto userid = userManager.verifyTokenAndGetUserId(*msgPt, fd);
    if(userid)
    {
        RoomIDType roomid = msgPt->body["args"][0].asUInt();
        if(roomManager.verifyRoomExist(roomid, fd) && roomManager.verifyUserInRoom(roomid, userid, fd))
        {
            if(cmd_name == CMD_CHAT_SEND)
            {
                std::string text = msgPt->body["args"][1].asString();
                const auto& username = userManager.getUser(userid)->getUserName();
                ChatItem item{userid, username, text};
                auto roomPt = roomManager.getRoomPt(roomid);
                roomPt->addChatItem(item);
                return;
            }
            if(cmd_name == CMD_CHAT_GET)
            {
                auto roomPt = roomManager.getRoomPt(roomid);
                time_t timestamp = msgPt->body["args"][1].asUInt();
                uint index = msgPt->body["args"][2].asUInt();
                roomPt->sendUnread(timestamp, index, fd);
                return;
            }
        }
    }
}
