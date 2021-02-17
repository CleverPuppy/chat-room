#pragma once
#include "sys/epoll.h"
#include "netutils.h"
#include "cproto.h"
#include "cprotomsg.h"
#include "usermanager.h"
#include "people.h"
#include <list>
#include <unordered_map>
#include <memory>

class Server
{
public:
    Server();
    ~Server();
    void run();
private:
    using UserID = uint64_t;
    using UserToken = uint64_t;
    int listen_sock;
    int epollfd;
    static const int MAX_EVENTS = 10;
    struct epoll_event ev, events[Server::MAX_EVENTS];
    
    CProtoMsgManager msgManager;
    UserManager userManager;
};
