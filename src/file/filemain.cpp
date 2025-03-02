#include <iostream>
#include <signal.h>
#include "chatserver.h"
#include "fileserver.h"
#include "chatservice.h"
using namespace std;

int main()
{
    EventLoop loop;
    InetAddress addr("0.0.0.0", 6001);
    FileServer server(&loop, addr, "FileServer");
    server.start();
    loop.loop();
    return 0;
}