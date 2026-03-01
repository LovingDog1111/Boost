#include "TrapTPA.h"

#include <Minecraft/ActorUtils.h>
#include <Minecraft/TargetUtil.h>
#include "../../SDK/GlobalInstance.h"
#include "../../SDK/NetWork/Packets/CommandRequestPacket.h"

TrapTPA::TrapTPA() : Module("TrapTPA", "Send TPA to low health targets", Category::COMBAT) {
    registerSetting(
        new SliderSetting<float>("Health Threshold", "Send tpa when target's health reaches this threshold", &healthThreshold, 10.f, 1.f, 20.f));
}



void TrapTPA::sendTPARequest(const std::string& playerName) {
    std::shared_ptr<Packet> packet = MinecraftPacket::createPacket(PacketID::CommandRequest);
    auto* pkt = reinterpret_cast<CommandRequestPacket*>(packet.get());

    pkt->mCommand = "/tpa " + playerName;
    pkt->mInternalSource = false;
    pkt->mOrigin.mType = CommandOriginType::Player;
    pkt->mOrigin.mPlayerId = 0;
    pkt->mOrigin.mRequestId = "0";
    pkt->mOrigin.mUuid = mce::UUID();

    auto client = GI::getClientInstance();
    if(!client || !client->packetSender)
        return;

    client->packetSender->sendToServer(pkt);
    Game.DisplayClientMessage(("Target low, sent TPA request to " + playerName).c_str());
}

void TrapTPA::onNormalTick(LocalPlayer* localPlayer) {
    if(!localPlayer)
        return;

    auto actors = ActorUtils::getActorList(true);
    for(auto* actor : actors) {
        if(!TargetUtil::isTargetValid(actor, false))
            continue;
        if(actor->getHealth() > healthThreshold)
            continue;

        std::string name = actor->getNameTag();
        if(sentTargets.find(name) != sentTargets.end())
            continue;

        sendTPARequest(name);
        sentTargets.insert(name);
    }
}
