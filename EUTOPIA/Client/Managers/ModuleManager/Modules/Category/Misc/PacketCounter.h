#pragma once
#include "../../ModuleBase/Module.h"

class PacketCounter : public Module {
   private:
    int packetCount = 0;
    int rendermode;
    int incomingCount = 0;
    int outgoingCount = 0;
    int counterMode = 0;  


   public:
    PacketCounter();

    void onSendPacket(Packet* packet) override;
    void onMCRender(MinecraftUIRenderContext* ctx) override;
    void onDisable() override;
    void onD2DRender() override;
    void onRecievePacket(Packet* packet, bool* cancel) override;
};
