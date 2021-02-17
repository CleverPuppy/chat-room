#pragma once
#include <string>
#include <vector>
#include <memory>
#include <list>
#include "room.h"
#include "cproto.h"

using UserId = uint64_t;
using UserToken = std::string;
using UserName = std::string;
using UserPassword = std::string;

struct PeopleProfile
{
    UserName user_name;
    UserPassword password;
    UserId user_id;
    UserToken token;

    PeopleProfile(const UserName& username, 
           const UserPassword& password,
           const UserId& ID,
           const UserToken& token)
        : user_name(username), password(password), user_id(ID), token(token) {}
};

class People
{
public:
    PeopleProfile profile;
    std::list<std::shared_ptr<Room>> inRooms;

    People(const UserName& username, 
           const UserPassword& password,
           const UserId& ID,
           const UserToken& token = "")
        : profile(username, password, ID, token) {}
    ~People() = default;

    const UserId& getUserID(){ return profile.user_id;}
    const UserName& getUserName() { return profile.user_name;}
    const UserToken& getUserToken() {return profile.token;}
    const UserPassword& getUserPassword() {return profile.password;}

    void updateToken(const UserToken& token){ profile.token = token;}
};
