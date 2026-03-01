#pragma once
#include "../../ModuleBase/Module.h"

class Crits : public Module {
   private:
    PlayerAuthInputPacket lastInputPacket{};

    void sendPacketAsync(const PlayerAuthInputPacket& pkt);
    void setPrevAuthInputPacket(const PlayerAuthInputPacket& pkt);

   public:
    Crits();
    bool lb = false;
    void onSendPacket(Packet* packet) override;
    void onAttack(Actor* target, bool& cancel);
};