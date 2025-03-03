#include <iostream>
#include <signal.h>
#include "chatserver.h"
#include "fileserver.h"
#include "chatservice.h"
#include <sys/stat.h>
using namespace std;

static int SwitchDeamon() {
    pid_t ret = fork();
    if (ret == -1)return -1;
    if (ret > 0)exit(0);//主进程到此为止
    //子进程内容如下
    ret = setsid();
    if (ret == -1)return -2;//失败，则返回
    ret = fork();
    if (ret == -1)return -3;
    if (ret > 0)exit(0);//子进程到此为止
    //孙进程的内容如下，进入守护状态
    for (int i = 0; i < 3; i++) close(i);
    umask(0);
    signal(SIGCHLD, SIG_IGN);
    return 0;
}

int main()
{
    SwitchDeamon();
    EventLoop loop;
    InetAddress addr("0.0.0.0", 6000);
    ChatServer server(&loop, addr, "ChatServer");
    server.start();
    loop.loop();
    return 0;
}