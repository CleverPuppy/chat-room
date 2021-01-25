#include "people.h"
#include <unistd.h>

void People::readFromFd()
{
    printf("read form %d",fd);
    char buffer[2048];
    int read_num = 0;
    while (( read_num = read(fd, buffer, sizeof(buffer))) > 0)
    {
        decoder.parser(buffer, read_num);
    }
    if(read_num == -1 && errno != EAGAIN)
    {
        perror("read error\n");
    }
}

std::shared_ptr<CProtoMsg> People::popMsg() 
{
    if(decoder.empty())
    {
        return {};
    }
    auto&& msg = decoder.front();
    decoder.pop();
    return msg;
}
