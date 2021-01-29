#pragma once
#ifndef __CLIENT_H__
#define __CLIENT_H__
#include "people.h"
#include "status.h"

class Client
{
public:
    Client();
private:
    ClientStatus status;
};
#endif // __CLIENT_H__