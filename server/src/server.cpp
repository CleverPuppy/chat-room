#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "sys/epoll.h"
#include "stdio.h"

#include "server.h"
#include "netutils.h"
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
                addPeople(conn_sock);
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
                auto peoplePt = getPeoplePt(incommingFd);
                // TODO read data
                if(peoplePt)
                {
                    peoplePt->readFromFd();
                    while (auto msgPt = peoplePt->popMsg())
                    {
                        std::cout << msgPt->body << std::endl;
                    }
                }else{
                    printf("peoplePt is nullptr\n");
                }
            }
        }
    }
}

inline decltype(Server::peoples)::iterator  Server::getPeople(int fd) 
{
    return peopleMap[fd];
}

inline void Server::addPeople(int fd) 
{
    peoples.emplace_front(new People(fd));
    if(peopleMap.count(fd) != 0)
    {
        removePeople(fd);
    }
    peopleMap.insert({fd, peoples.begin()});
}

inline void Server::removePeople(int fd) 
{
    if(peopleMap.count(fd) != 0)
    {
        auto it = getPeople(fd);
        peoples.erase(it);
    }
}

inline std::shared_ptr<People> Server::getPeoplePt(int fd) 
{
    return *getPeople(fd);
}
