
#pragma once

#include <chrono>

#include "Packet.h"

class NetworkStackLatencyPacket : public ::Packet {
   public:
    static inline PacketID ID = PacketID::NetworkStackLatency;
    uint64_t mCreateTime;  // this+0x30
    bool mFromServer;      // this+0x38
};