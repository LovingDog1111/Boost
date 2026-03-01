#include "AntiCrystal.h"

AntiCrystal::AntiCrystal()
    : Module("AntiCrystal", "AntiCrystal", Category::PLAYER) {
    registerSetting(
        new SliderSetting<float>("reduce", "How deep you hide", &reduce, reduce,  0.1f,1.f));
}

void AntiCrystal::onSendPacket(Packet* packet) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;
    const Vec3<float>& actorPos = localPlayer->getPos();
    if(packet->getId() == PacketID::PlayerAuthInput) {
        GI::DisplayClientMessage("Identify packet authinput");
        PlayerAuthInputPacket* authPkt = (PlayerAuthInputPacket*)packet;
        authPkt->mPos.y = (actorPos.y - reduce);
    }
    if(packet->getId() == PacketID::MovePlayer) {
        GI::DisplayClientMessage("Identify packet MovePlayer");
        MovePlayerPacket* movePkt = (MovePlayerPacket*)packet;
        movePkt->mPos.y = (actorPos.y - reduce);
    }
}

void AntiCrystal::onReceivePacket(Packet* packet, bool* cancel) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;
    const Vec3<float>& actorPos = localPlayer->getPos();
    if(packet->getId() == PacketID::PlayerAuthInput) {
        GI::DisplayClientMessage("Identify packet authinput");
        PlayerAuthInputPacket* authPkt = (PlayerAuthInputPacket*)packet;
        authPkt->mPos.y = (actorPos.y - reduce);
    }
    if(packet->getId() == PacketID::MovePlayer) {
        GI::DisplayClientMessage("Identify packet MovePlayer");
        MovePlayerPacket* movePkt = (MovePlayerPacket*)packet;
        movePkt->mPos.y = (actorPos.y - reduce);
    }
}
