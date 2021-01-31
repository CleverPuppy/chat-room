#include "cproto.h"
#include <iostream>

int main()
{
    Json::Value root;
    std::cin >> root;
    std::cout << root;

    CProtoMsg msg{};
    msg.body = root;
    // msg.body.append(root);
    uint32_t len = 0;
    CProtoEncoder encoder{};
    auto pMsgEncoded = encoder.encode(&msg, len);
    
    CProtoDecoder decoder{};
    decoder.init();
    decoder.parser((void*)pMsgEncoded, len);
    std::cout << decoder.front()->body;
    return 0;
}