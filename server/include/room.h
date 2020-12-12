#pragma once
#include <list>
#include <memory>

#include "people.h"

enum class ROOM_STATES
{
    OPEN,
    LOCKED,
    CLOSED
};

class Room
{
public:
    Room() = default;
    ~Room() = default;

private:
    ROOM_STATES states;
    std::list<std::shared_ptr<People>> managers;
    std::list<std::shared_ptr<People>> menbers; // contains managers
};
