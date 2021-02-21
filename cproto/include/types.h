#pragma once
#include <string>
#include <memory>
#include <unistd.h>
#include "cproto.h"

using UserId = uint64_t;
using UserToken = std::string;
using UserName = std::string;
using UserPassword = std::string;

using RoomIDType = uint32_t;
using RoomNameType = std::string;

using MsgPtType = std::shared_ptr<CProtoMsg>;