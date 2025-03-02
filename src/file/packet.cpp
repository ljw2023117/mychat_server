#include "packet.h"

CPacket::CPacket(string id, uint64_t sum_size, bool f, uint16_t index, uint16_t sum, uint64_t size, char *_data)
    : sHead(0xFEFF), file_id(id), file_size(sum_size), flag(f),chunk_index(index), chunk_sum(sum), data_size(size), data(_data)
{
}

CPacket::CPacket(const char *pData) {
    size_t i = 2;   // head 2字节
    file_id = string(pData + 2, 32);    // 32字节
    file_size = *(uint64_t*)(pData + 34); // 8字节
    flag = *(bool*)(pData + 42);    // 1字节
    chunk_index = *(uint16_t*)(pData + 43);   // 2字节
    chunk_sum = *(uint16_t*)(pData + 45);     // 2字节
    data_size = *(uint64_t*)(pData + 47);    // 8字节
    data = pData + 55;
}

string CPacket::Data() {
    string msg;
    msg += to_string(sHead);
    msg += file_id;
    msg += to_string(file_size);
    msg += to_string(chunk_index);
    msg += to_string(chunk_sum);
    msg += to_string(data_size);
    msg.append(data, data_size);
    return msg;
}

void CPacket::text()
{
    cout << "sHead: " << sHead << endl;
    cout << "file_id: " << file_id << endl;
    cout << "file_size: " << file_size << endl;
    cout << "chunk_index: " << chunk_index << endl;
    cout << "chunk_sum: " << chunk_sum << endl;
    cout << "data_size: " << data_size << endl;
}

// CPacket::CPacket(const char *pData, size_t &nSize) {
//     size_t i = 0;
//     for (; i < nSize; i++)
//     {
//         if (*(uint16_t*)(pData + i) == 0xFEFF)
//         {
//             sHead = *(uint16_t*)(pData + i);
//             i += 2;
//             break;
//         }
//     }
//     if (i + 4 + 2 + 2 > nSize) // 包数据可能不全，或者包头未能全部接收到
//     {
//         nSize = 0;
//         return;
//     }

//     nLength = *(int*)(pData + i); i += 4;
//     if (nLength + i > nSize) // 包未完全接收到，就返回，解析失败
//     {
//         nSize = 0;
//         return;
//     }
//     sCmd = *(short*)(pData + i); i += 2;
//     if (nLength > 4)
//     {
//         strData.resize(nLength - 2 - 2);
//         memcpy((void*)strData.c_str(), pData + i, nLength - 4);
//         i += nLength - 4;
//     }
//     sSum = *(short*)(pData + i); i += 2;

//     short sum = 0;
//     for (int j = 0; j < strData.size(); j++)
//     {
//         sum += char(strData[j]) & 0xFF;
//     }
//     if (sum == sSum)
//     {
//         nSize = i;
//         return;
//     }
//     nSize = 0;
// }



