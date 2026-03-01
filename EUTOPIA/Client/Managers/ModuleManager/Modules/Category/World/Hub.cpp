#include "Hub.h"

#include "../../SDK/GlobalInstance.h"
#include "../../SDK/NetWork/Packets/CommandRequestPacket.h"

Hub::Hub() : Module("Hub", "Teleport to the hub", Category::WORLD) {}

Hub::~Hub() {}

void sendHub() {
    std::shared_ptr<Packet> packet = MinecraftPacket::createPacket(PacketID::CommandRequest);
    auto* pkt = reinterpret_cast<CommandRequestPacket*>(packet.get());

    pkt->mCommand = "/hub";
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

void Hub::onEnable() {
    sendHub();
    this->setEnabled(false);
}
