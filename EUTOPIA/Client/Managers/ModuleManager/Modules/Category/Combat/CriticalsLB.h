#pragma once

#include "../../ModuleBase/Module.h"
#include "../../SDK/Network/Packets/MovePlayerPacket.h"
#include "../../SDK/Network/Packets/PlayerAuthInputPacket.h"

class CriticalsLB : public Module {
   public:
    CriticalsLB();
    void onSendPacket(Packet* packet) override;
    void onAttack(Actor* target, bool& cancel);

   private:
    PlayerAuthInputPacket lastInputPacket;
    void sendPacketAsync(const PlayerAuthInputPacket& pkt);
    void setPrevAuthInputPacket(const PlayerAuthInputPacket& pkt);
    bool lb = true;
};
