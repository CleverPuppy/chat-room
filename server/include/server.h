#pragma once
#include "sys/epoll.h"

class Server
{
public:
    Server();
    ~Server();
private:
    int listen_sock;
    int epollfd;
    static const int MAX_EVENTS = 10;
    struct epoll_event ev, events[Server::MAX_EVENTS];
};
