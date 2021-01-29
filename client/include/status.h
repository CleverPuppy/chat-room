#pragma once

enum class ClientStatus{
    START,
    FAILED_CONNECT_TO_SERVER,
    WAITING_FOR_LOGING,
    LOGGING,
    REGISTERING,
    WAITING_FOR_CMD
};