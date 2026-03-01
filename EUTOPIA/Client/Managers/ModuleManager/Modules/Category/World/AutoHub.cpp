#include "AutoHub.h"

#include "../../SDK/GlobalInstance.h"
#include "../../SDK/NetWork/Packets/CommandRequestPacket.h"

AutoHub::AutoHub() : Module("AutoHub", "Auto /hub on low health", Category::WORLD) {
    registerSetting(new SliderSetting<int>("Health", "Minimum health to trigger /hub",
                                           &healthThreshold, 1, 1, 20));
}

void AutoHub::onEnable() {
    alreadyTeleported = false;
    GI::DisplayClientMessage("%s[AutoHub] Enabled", MCTF::GREEN);
}

void AutoHub::onDisable() {
    alreadyTeleported = false;
    GI::DisplayClientMessage("%s[AutoHub] Disabled", MCTF::RED);
}

void AutoHub::sendHub() {
    auto client = GI::getClientInstance();
    if(!client || !client->packetSender)
        return;

    std::shared_ptr<Packet> packet = MinecraftPacket::createPacket(PacketID::CommandRequest);
    if(!packet)
        return;

    auto* pkt = reinterpret_cast<CommandRequestPacket*>(packet.get());
    if(!pkt)
        return;

    pkt->mCommand = "/hub";
    pkt->mInternalSource = false;
    pkt->mOrigin.mType = CommandOriginType::Player;
    pkt->mOrigin.mPlayerId = 0;
    pkt->mOrigin.mRequestId = "0";
    pkt->mOrigin.mUuid = mce::UUID();

    GI::DisplayClientMessage("%s[AutoHub] Health low, teleporting to hub..", MCTF::RED);

    client->packetSender->sendToServer(pkt);

    GI::DisplayClientMessage("%s[AutoHub] Teleported to hub", MCTF::RED);
}

void AutoHub::onNormalTick(LocalPlayer* localPlayer) {
    if(!localPlayer)
        return;

    if(!localPlayer->level)
        return;

    float currentHealth = localPlayer->getHealth();
    if(currentHealth <= 0.f)
        return;

    if(currentHealth <= static_cast<float>(healthThreshold)) {
        if(!alreadyTeleported) {
            sendHub();
            alreadyTeleported = true;
        }
    } else {
        alreadyTeleported = false;
    }
}