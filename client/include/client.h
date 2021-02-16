#pragma once
#ifndef __CLIENT_H__
#define __CLIENT_H__
#include "people.h"
#include "status.h"
#include "login.h"

constexpr int MAX_RETRY_TIMES = 3;

class Client
{
public:
    Client();
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    ~Client();

    void mainLoop();
private:
    ClientStatus status;
    int fd;
    
    int retryTimes = 0;

    void connectToServer();
    void waitingForLogin();
    void waitingForLoginHint();
    void statusTransfrom();

    bool isCmdValid(const std::vector<std::string>& cmds);
    void wrongCmdSizeHint(const std::string& cmd, int real, int target);
    void wrongCmdNameHint(const std::string& cmd);
};
#endif // __CLIENT_H__