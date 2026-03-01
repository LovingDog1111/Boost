#pragma once

#include <cstdint>
#include <vector>

#include "../../ModuleBase/Module.h"
#include "../SDK/NetWork/Packets/InventoryTranscationPacket.h"
#include "../SDK/NetWork/Packets/NetworkStackLatencyPacket.h"
#include "../SDK/NetWork/Packets/PacketEvent.h"
#include "../SDK/NetWork/Packets/PlayerAuthInputPacket.h"
#include "../../../../../../SDK/NetWork/Packets/PacketInevent.h"

class Actor;

class DisablerNew : public Module {
   public:
    DisablerNew();
    void onReceivePacket(Packet* packet, bool* cancel) override;
    static DisablerNew* getInstance() {
        static DisablerNew instance;
        return &instance;
    }
   private:
    struct LatencyEntry {
        uint64_t timestamp;
        uint64_t sendAt;
    };

    int Mode;
    int targetPing;
    int jitterRange;

    // int64_t mClientTicks;
    bool mShouldUpdateClientTicks;

    Actor* mFirstAttackedActor;

    std::vector<LatencyEntry> mQueue;

};
