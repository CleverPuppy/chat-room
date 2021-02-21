#include "people.h"
#include <unistd.h>

void People::joinRoom(RoomIDType roomId) 
{
    rooms.insert(roomId);
}
