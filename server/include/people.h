#pragma once
#include <string>
#include <vector>
#include <memory>
#include <set>
#include "room.h"
#include "cproto.h"

class Room;

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
    void joinRoom(RoomIDType roomId);

    std::set<RoomIDType> rooms;
};
