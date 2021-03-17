# chat-room

## 介绍

简单的聊天室Demo

1. 服务端

    服务端采用单线程epool模式

2. 客户端

    客户端为命令行式应用，整体由两个线程执行。主线程执行UI的IO，子线程采用select服务端发送的消息

## 使用方法

### 编译

```
git clone https://github.com/CleverPuppy/chat-room.git
cd chat-room && mkdir build && cd build && cmake ../ && make
```

编译后，会生成多个二进制文件，其中

- build/bin/client   - 客户端
- build/bin/server   - 服务端

### 运行

假设当前目录为build

- 运行server端 ```./bin/server```
- 运行client端 ```./bin/client```

## TODO

- Server

    [x] 主动释放fd 

- Client

    [x] 尝试使用更现代的UI