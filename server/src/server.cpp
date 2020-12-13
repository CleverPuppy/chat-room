#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "sys/epoll.h"
#include "stdio.h"

#include "server.h"
#include "netutils.h"

Server::Server()
{
    listen_sock = NetUtils::create_socket();
    if (listen_sock != -1)
    {
        NetUtils::make_socket_unblock(listen_sock);
        NetUtils::bind_and_listen(listen_sock, "127.0.0.1", 8080);
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
            }else
            {
                /*
                 *                 TODO Read data
                 *                  do_use_fd(events[n].data.fd)
                */
            }
        }
    }
}

Server::~Server()
{
}