#include "../../SDK/GlobalInstance.h"
#include "../../SDK/NetWork/Packets/CommandRequestPacket.h"
#include "Spawn.h"

Spawn::Spawn() : Module("Spawn", "Teleport to the spawn (sm)", Category::WORLD) {}

Spawn::~Spawn() {}

void tptospawn() {
    std::shared_ptr<Packet> packet = MinecraftPacket::createPacket(PacketID::CommandRequest);
    auto* pkt = reinterpret_cast<CommandRequestPacket*>(packet.get());

    pkt->mCommand = "/spawn";
    pkt->mInternalSource = false;
    pkt->mOrigin.mType = CommandOriginType::Player;
    pkt->mOrigin.mPlayerId = 0;
    pkt->mOrigin.mRequestId = "0";
    pkt->mOrigin.mUuid = mce::UUID();

    auto client = GI::getClientInstance();
    if(!client || !client->packetSender)
        return;

    client->packetSender->sendToServer(pkt);
}

void Spawn::onEnable() {
    tptospawn();
    GI::DisplayClientMessage("[Spawn] Teleporting to spawn..");
    this->setEnabled(false);
}
