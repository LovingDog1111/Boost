#include "AutoHub.h"

#include "../../SDK/GlobalInstance.h"
#include "../../SDK/NetWork/Packets/CommandRequestPacket.h"

AutoHub::AutoHub() : Module("AutoHub", "Auto /hub on low health", Category::WORLD) {
    registerSetting(new SliderSetting<int>("Health", "Minimum health to trigger /hub",
                                           &healthThreshold, 1, 1, 20));
}


void AutoHub::onEnable() {
    GI::DisplayClientMessage("%s[AutoHub] Enabled", MCTF::GREEN);
}

void AutoHub::onDisable() {
    GI::DisplayClientMessage("%s[AutoHub] Disabled", MCTF::RED);
}




 void AutoHub::sendHub() {
    GI::DisplayClientMessage("%s[AutoHub] Health low, teleporting to hub..", MCTF::RED);
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
    GI::DisplayClientMessage("%s[AutoHub] Teleported to hub", MCTF::RED);
 }

void AutoHub::onNormalTick(LocalPlayer* localPlayer) {
    if(!localPlayer || localPlayer->getHealth() <= 0)
        return;

    float currentHealth = localPlayer->getHealth();

    if(currentHealth <= healthThreshold) {
        if(!alreadyTeleported) {
            sendHub();
            alreadyTeleported = true;
        }
    } else {
        alreadyTeleported = false;
    }
}
