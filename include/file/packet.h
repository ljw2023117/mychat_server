#include <iostream>
#include <string>

using namespace std;

class CPacket
{
public:
    CPacket();
    CPacket(string id, uint64_t sum_size, bool f, uint16_t index, uint16_t sum, uint64_t size, char* _data);
    CPacket(const char* pData);  // 构造函数，自动解包，nSize传入传出参数，记录使用了多少数据
    CPacket(const CPacket& pack);               // 拷贝构造
    string Data();
    void text();
    CPacket& operator=(const CPacket& pack);    // 赋值构造

    string get_file_id(){return file_id;}
    uint64_t get_file_size(){return file_size;}
    bool getflag(){return flag;}
    uint16_t get_chunk_index(){return chunk_index;}
    uint16_t get_chunk_sum(){return chunk_sum;}
    uint64_t get_data_size(){return data_size;}
    const char* get_data(){return data;}


private:
    uint16_t sHead;            // 固定位 0xFEFF
    /* 文件元数据区 (16字节) */
    string file_id;        // 文件唯一标识md5 32字节
    uint64_t file_size;      // 文件总大小
    bool flag;              // 0 上传文件 1 下载文件
    /* 分片控制区 4字节 */
    uint16_t chunk_index;   // 当前分片序号
    uint16_t chunk_sum;     // 分片总数
    uint64_t data_size;    // 当前分片大小（动态调整）
    const char* data;
};