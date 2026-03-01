#pragma once
#include <memory>
#include <random>
#include <vector>

#include "../../ModuleBase/Module.h"

class Disabler : public Module {
   public:
    Disabler();

    std::string getModeText() override;
    void onSendPacket(Packet* packet) override;
    void onNormalTick(LocalPlayer* player) override;

   private:
    int Mode = 0;
    bool sentinelAllowed = false;
    bool sentinelChecked = false;
    bool debugMessages = false;

    int targetPing = 100;      
    float jitterRange = 10.f;  

    struct QueuedLatencyPacket {
        uint64_t originalTimestamp;
        uint64_t sendTime;
    };

    std::vector<QueuedLatencyPacket> mPacketQueue;

    void processLatencyQueue();
};
