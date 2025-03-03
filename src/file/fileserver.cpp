#include "fileserver.h"
#include "json.hpp"
#include <iostream>
#include <fcntl.h>
#include "chatservice.h"
#include <sys/stat.h>
#include "CTool.h"

using json = nlohmann::json;

FileServer::FileServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg)
    :_server(loop, listenAddr, nameArg)
{
    // 注册链接回调
    _server.setConnectionCallback(std::bind(&FileServer::onConnection, this, _1));
    // 注册消息回调
    _server.setMessageCallback(std::bind(&FileServer::onMessage, this, _1, _2, _3));

    // 设置线程数量
    _server.setThreadNum(0);
}

void FileServer::recvfile(CPacket &pack) {
    string filename = "../filecache/" + pack.get_file_id();

    // 打开文件
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1)
    {
        cout << "文件打开失败" << endl;
        return;
    }

    // 写入数据
    ssize_t bytes_written = write(fd, pack.get_data(), pack.get_data_size());
    if (bytes_written == -1) {
        cout << "文件写入失败" << endl;
        close(fd);
        return;
    }

    // 关闭文件
    close(fd);
    printf("成功追加 %zd 字节到文件\n", bytes_written);
}

void FileServer::sendfile(const TcpConnectionPtr& conn, CPacket &pack) {
    string filename = "../filecache/" + pack.get_file_id();

    int buffer_size = 64 * 1024;

    // 定义缓冲区
    char buffer[buffer_size];
    ssize_t bytes_read;

    // 打开文件
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1)
    {
        cout << "文件打开失败" << endl;
        return;
    }
    // 获取文件信息
    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        perror("获取文件信息失败");
        close(fd);
        return;
    }

    off64_t file_size = file_stat.st_size;
    int shard = 1;
    int total_shard = file_size / buffer_size + 1;
    // 循环读取发送数据
    while ((bytes_read = read(fd, buffer, buffer_size)) > 0) {
        CPacket pack_(pack.get_file_id(), file_size, 1, shard, total_shard, bytes_read, buffer);
        // string* data = new string(pack_.Data());
        // string data(pack_.Data());
        auto dataPtr = std::make_shared<std::string>(std::string(pack_.Data()));
        shard += 1;
        conn->getLoop()->runInLoop([conn, dataPtr]() {
            // CTool::Dump(dataPtr->data(), dataPtr->size());
            usleep(60000);
            conn->send(dataPtr->data(), dataPtr->size());
            printf("成功发送 %zd 字节文件\n", dataPtr->size());
        });
    }

    // 关闭文件
    close(fd);
}

void FileServer::start() {
    _server.start();
}

void FileServer::onConnection(const TcpConnectionPtr& conn)
{
    // 客户端断开链接
    if (!conn->connected()){
        conn->shutdown();
    }
}

void FileServer::onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time)
{
    cout << "FileServer::onMessage==" << endl;
    const char* data = buffer->peek();
    int size = buffer->readableBytes();
    if (*(uint16_t*)data != 0xFEFF)
    {
        cout << "FileServer onMessage : head error" << endl;
        return;
    }
    CPacket pack(data);
    int packsize = pack.get_data_size() + 55; 
    cout << "buffer size: " << size << endl;
    cout << "当前包的大小：" << packsize << endl;
    while (!pack.getflag() && size >= packsize && packsize > 55)
    {
        buffer->retrieve(packsize);
        recvfile(pack);
        data = buffer->peek();
        if (*(uint16_t *)data != 0xFEFF) {
            cout << "FileServer onMessage : head error" << endl;
            return;
        }
        CPacket pack(data);
        size = buffer->readableBytes();
        packsize = pack.get_data_size() + 55;
    }
    if (pack.getflag() && size >= packsize)
    {
        buffer->retrieve(packsize);
        sendfile(conn, pack);
    }
}
