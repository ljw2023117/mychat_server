#include <iostream>
#include <signal.h>
#include "chatserver.h"
#include "chatservice.h"
using namespace std;

int main()
{
    EventLoop loop;
    InetAddress addr("0.0.0.0", 6000);
    ChatServer server(&loop, addr, "ChatServer");
    server.start();
    loop.loop();
    return 0;
}