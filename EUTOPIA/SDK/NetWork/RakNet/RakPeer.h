#pragma once
#include "../../Utils/MemoryUtil.h"
#include <stdint.h>


class RakPeer {
public:
    bool Ping(const char* addy, unsigned short port, bool b1, unsigned int u1) {
     return MemoryUtil::callVirtualFunc<bool>(38,this , addy, port, b1, u1);
    }

    int GetAveragePing(std::string const& addressOrGuid) {
        return MemoryUtil::callVirtualFunc<int>(41,this, addressOrGuid);  // getlastping
    }
};
