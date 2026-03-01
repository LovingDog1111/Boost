#include "Crits.h"

Crits::Crits() : Module("Crits", "Each hit is a critical hit", Category::COMBAT) {
    registerSetting(new BoolSetting("LifeBoat", "Bypasses lifeboat kick", &lb, true));
}

void Crits::sendPacketAsync(const PlayerAuthInputPacket& pkt) {
    if(!Game.canUseMoveKeys())
        return;

    auto packet = MinecraftPacket::createPacket(PacketID::PlayerAuthInput);
    if(!packet)
        return;

    auto* newPkt = reinterpret_cast<PlayerAuthInputPacket*>(packet.get());
    if(!newPkt)
        return;

    *newPkt = pkt;

    if(lb) {
        newPkt->TicksAlive = 0;
    }

    LocalPlayer* localPlayer = Game.getLocalPlayer();
    if(!localPlayer || !localPlayer->level || !GI::getPacketSender())
        return;

    GI::getPacketSender()->sendToServer(newPkt);
}

void Crits::setPrevAuthInputPacket(const PlayerAuthInputPacket& pkt) {
    if(!Game.canUseMoveKeys())
        return;
    lastInputPacket = pkt;
}

void Crits::onSendPacket(Packet* packet) {
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

void Crits::onAttack(Actor* target, bool& cancel) {
    if(!Game.canUseMoveKeys())
        return;

    if(lastInputPacket.TicksAlive < 5)
        return;

    PlayerAuthInputPacket pktToModify;
    pktToModify = lastInputPacket;

    //pktToModify.removeMovingInput();

    pktToModify.mInputData |= AuthInputAction::JUMPING;
    pktToModify.mInputData |= AuthInputAction::JUMP_DOWN;
    pktToModify.mPos.y += 1.0f;

    if(lb) {
        pktToModify.TicksAlive = 0;
    }

    sendPacketAsync(pktToModify);

    pktToModify.mInputData &= ~AuthInputAction::JUMPING;
    pktToModify.mInputData &= ~AuthInputAction::JUMP_DOWN;
    pktToModify.mPos.y -= 0.5f;

    for(int i = 0; i < 6; i++) {
        if(lb) {
            pktToModify.TicksAlive = 0;
        }
        sendPacketAsync(pktToModify);
    }
}
