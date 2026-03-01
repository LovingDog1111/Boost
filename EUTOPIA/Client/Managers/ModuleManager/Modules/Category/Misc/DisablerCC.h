#pragma once
#include "../../ModuleBase/Module.h"

class DisablerCC : public Module {
   public:
    DisablerCC();

    void onEnable() override;
    void onDisable() override;
    void onSendPacket(Packet* packet) override;
    void onNormalTick(LocalPlayer* player) override;
    void onReceivePacket(Packet* packet, bool* cancel) override;
    void onMCRender(MinecraftUIRenderContext* ctx) override;
    void onD2DRender() override;

    struct QueuedLatencyPacket {
        uint64_t originalTimestamp;
        uint64_t sendTime;
    };

    std::vector<QueuedLatencyPacket> mPacketQueue;
        
   private:
    int lastPing = 0; 
    int Mode = 0;
    float targetPing = 100;
    void processLatencyQueue();
    bool movementSpoof;
    bool showping;
    int rendermode;

};
