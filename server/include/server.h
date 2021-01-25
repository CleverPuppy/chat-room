#pragma once
#include "sys/epoll.h"
#include "netutils.h"
#include "cproto.h"
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
    int listen_sock;
    int epollfd;
    static const int MAX_EVENTS = 10;
    struct epoll_event ev, events[Server::MAX_EVENTS];
    CProtoEncoder encoder;
    CProtoDecoder decoder;

    std::list<std::shared_ptr<People>> peoples;
    std::unordered_map<int,decltype(peoples)::iterator> peopleMap;

    decltype(peoples)::iterator getPeople(int fd);
    std::shared_ptr<People> getPeoplePt(int fd);
    void addPeople(int fd);
    void removePeople(int fd);
};
