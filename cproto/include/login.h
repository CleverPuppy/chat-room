#pragma once

#include "cproto.h"
#include "cprotomsg.h"
#include <string>

class People;
constexpr const char* LOGIN_CMD = "login";
constexpr const char* REGISTER_CMD = "register";

class Login
{
public:
    Login() = delete;
    static bool login(const std::string& username, const std::string& password, int fd);
    static bool registerAndLogin(const std::string& username, const std::string& password, int fd);
    static std::string crptPassword(const std::string& password);
};
