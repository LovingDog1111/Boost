#include "CriticalsLB.h"

CriticalsLB::CriticalsLB() : Module("CriticalsLB", "Each hit is a critical hit", Category::COMBAT) {
    registerSetting(new BoolSetting("LifeBoat", "Bypasses lifeboat kick", &lb, true));
}

void CriticalsLB::sendPacketAsync(const PlayerAuthInputPacket& pkt) {
    if(!Game.canUseMoveKeys())
        return;

    auto packet = MinecraftPacket::createPacket(PacketID::PlayerAuthInput);
    auto* newPkt = reinterpret_cast<PlayerAuthInputPacket*>(packet.get());

    *newPkt = pkt;

    if(lb) {
        newPkt->TicksAlive = 0;
    }

    LocalPlayer* localPlayer = Game.getLocalPlayer();
    if(!localPlayer || !localPlayer->level || !Game.getPacketSender())
        return;

    Game.getPacketSender()->sendToServer(newPkt);
}

void CriticalsLB::setPrevAuthInputPacket(const PlayerAuthInputPacket& pkt) {
    if(!Game.canUseMoveKeys())
        return;
    lastInputPacket = pkt;
}

void CriticalsLB::onSendPacket(Packet* packet) {
    if(!Game.getLocalPlayer())
        return;
    if(!Game.canUseMoveKeys())
        return;

    if(packet->getId() == PacketID::MovePlayer) {
        auto& movePkt = *reinterpret_cast<MovePlayerPacket*>(packet);
        movePkt.mOnGround = false;
    } else if(packet->getId() == PacketID::PlayerAuthInput) {
        const auto& currAuthPkt = *reinterpret_cast<PlayerAuthInputPacket*>(packet);
        setPrevAuthInputPacket(currAuthPkt);
    }
}

void CriticalsLB::onAttack(Actor* target, bool& cancel) {
    if(!Game.canUseMoveKeys())
        return;

    if(lastInputPacket.TicksAlive < 5)
        return;

    PlayerAuthInputPacket pktToModify = lastInputPacket;

    pktToModify.removeMovingInput();

    pktToModify.mInputData &= ~AuthInputAction::JUMPING;
    pktToModify.mInputData &= ~AuthInputAction::JUMP_DOWN;

    pktToModify.mPos.y -= 1.f;


    if(lb) {
        pktToModify.TicksAlive = 0;
    }

    sendPacketAsync(pktToModify);

    if(lb) {
        pktToModify.TicksAlive = 0;
    }

    pktToModify.mInputData &= ~AuthInputAction::JUMPING;
    pktToModify.mInputData &= ~AuthInputAction::JUMP_DOWN;

    pktToModify.mPos.y -= 0.5f;


    for(int i = 0; i < 6; i++) {
        sendPacketAsync(pktToModify);
    }
}
