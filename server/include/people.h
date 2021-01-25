#pragma once
#include <string>
#include <vector>
#include <memory>
#include <list>
#include "room.h"
#include "cproto.h"

struct PeopleProfile
{
    std::string user_name;
    std::uint64_t user_id;
};

class People
{
private:
    int fd;
    PeopleProfile profile;
    std::list<std::shared_ptr<Room>> inRooms;
    CProtoDecoder decoder;
public:
    People(int _fd): fd(_fd) {}
    ~People() = default;

    void readFromFd();
    std::shared_ptr<CProtoMsg> popMsg();
};
