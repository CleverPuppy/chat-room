#pragma once
#ifndef __CLIENT_H__
#define __CLIENT_H__
#include "people.h"
#include "status.h"
#include "loggin.h"

class Client
{
public:
    Client();
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    ~Client();
private:
    ClientStatus status;
    int fd;

    void connectToServer();
    void waitingForLoggin();
};
#endif // __CLIENT_H__