#pragma once
#pragma once
#include "../../ModuleBase/Module.h"
#include "../../../../../../SDK/NetWork/Packets/PlayerAuthInputPacket.h"

class AntiCrystal : public Module {
   private:
    float reduce = 0.6f;

   public:
    AntiCrystal();
    //    : Module("AntiCrystal", "Uses packets to phase your bottom half", Category::PLAYER) {
    //    registerSetting(
    //        new SliderSetting<float>("Reduce", "Amount to reduce", &reduce, reduce, 0.1f, 1.f));
    //}
    //virtual void onSendPacket(Packet* packet, bool& cancel) {
    //    LocalPlayer* localPlayer = GI::getLocalPlayer();
    //    if(!localPlayer)
    //        return;
    //    const Vec3<float>& actorPos = localPlayer->getPos();
    //    if(packet->getId() == PacketID::PlayerAuthInput) {
    //        PlayerAuthInputPacket* authPkt = (PlayerAuthInputPacket*)packet;
    //        authPkt->mPos.y = (actorPos.y - reduce);
    //    }
    //    if(packet->getId() == PacketID::MovePlayer) {
    //        MovePlayerPacket* movePkt = (MovePlayerPacket*)packet;
    //        movePkt->mPos.y = (actorPos.y - reduce);
    //    }
    //}
    void onSendPacket (Packet*packet) override;
    void onReceivePacket(Packet* packet,bool* cancel) override;
};