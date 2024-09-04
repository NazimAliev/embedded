#include <cassert>
#include <iostream>
#include <vector>
#include "dlt_client.h"

/***** ARGVTOBUFFER *****/

// argvMessageId template is SID:CID
int argvToBuffer(const std::string argvMessageId, const std::string argvData,
                 std::vector<uint8_t>& out)
{
    uint16_t sid;
    uint16_t cid;
    try
    {
        // getting sid, cid
        // std::cout << argvMessageId << std::endl;
        auto pos = argvMessageId.find(":");
        sid = std::stoul(argvMessageId.substr(0, pos), nullptr, 16);
        // std::cout << "sid: " << sid << std::endl;
        cid = std::stoul(argvMessageId.substr(pos + 1, argvMessageId.size()),
                         nullptr, 16);
        // std::cout << "cid: " << cid << std::endl;
    }
    catch (...)
    {
        std::cout << "Uncorrect data" << std::endl;
        return -1;
    }
    // pack 16-bit sid, cid to 8-bit buffer
    uint8_t* ptr;
    ptr = reinterpret_cast<uint8_t*>(&sid);
    out.insert(out.end(), ptr, ptr + sizeof(uint16_t));
    ptr = reinterpret_cast<uint8_t*>(&cid);
    out.insert(out.end(), ptr, ptr + sizeof(uint16_t));
    std::swap(out[0], out[1]);
    std::swap(out[2], out[3]);

    auto len = argvData.size();
    if (len % 2 != 0)
    {
        return -1;
    }
    try
    {
        // std::cout << "hex: " << std::endl;
        for (size_t i = 0, pos = 0; i < len / 2; ++i, pos += 2)
        {
            // std::cout << argvData << " : " << i << " -> " << pos <<
            // std::endl;
            auto sub = argvData.substr(pos, 2);
            // std::cout << "sub: " << sub << std::endl;
            uint8_t hex = std::stoul(sub, nullptr, 16);
            out.push_back(hex);
            // std::cout << static_cast<int>(hex) << std::endl;
        }
        // std::cout << std::endl;
    }
    catch (...)
    {
        std::cout << "Uncorrect data" << std::endl;
        return -1;
    }
    return 0;
}

/***** BUFFERTOMSG *****/

int bufferToMsg(const uint8_t* buffer, const size_t bufferSize, uint16_t& sid,
                uint16_t& cid, std::vector<uint8_t>& data)
{
    assert(bufferSize >= 18 && "BufferToMsg: buffer size less than 18");

    sid = buffer[4] | buffer[5] << 8;
    cid = buffer[10] | buffer[11] << 8;
    data.clear();
    data.insert(data.end(), buffer + 18, buffer + bufferSize);
    return 0;
}
