#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "sys/epoll.h"
#include "stdio.h"
#include "unistd.h"

#include "server.h"
#include "netutils.h"
#include "login.h"
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
                /*
                 *                 TODO Read data
                 *                  do_use_fd(events[n].data.fd)
                */
                int incommingFd = events[n].data.fd;
                #ifndef NDEBUG
                printf("Reading from %d\n", incommingFd);
                #endif
                msgManager.readData(incommingFd);
                while (auto msgPt = msgManager.getMsg(incommingFd))
                {
                    std::cout << msgPt->body << std::endl;
                    if(msgPt->body["cmd"] == LOGIN_CMD || msgPt->body["cmd"] == REGISTER_CMD)
                    {
                        auto args = msgPt->body["args"];
                        if(args.isArray())
                        {
                            if(args.size() != 2)
                            {
                                fprintf(stderr, "wrong args size\n");
                                break;
                            }
                            std::string username = args[0].asString();
                            std::string password = args[1].asString();
                            #ifndef NDEBUG
                            fprintf(stdout, "username : %s, password: %s\n", username.c_str(), password.c_str());
                            #endif
                            if(msgPt->body["cmd"] == LOGIN_CMD)
                            {
                                userManager.loginUser(incommingFd, username, password);
                            }
                            if(msgPt->body["cmd"] == REGISTER_CMD)
                            {
                                userManager.registerAndLoginUser(incommingFd, username, password);
                            }
                        }else
                        {
                            fprintf(stderr, "invalid request\n");
                        }
                    }
                }
            }
        }
    }
}
