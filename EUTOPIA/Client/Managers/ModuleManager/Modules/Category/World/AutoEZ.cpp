#include "AutoEZ.h"
#include "../Utils\Minecraft\TargetUtil.h"
#include <unordered_map>
#include "..\SDK\NetWork\Packets\TextPacket.h"

std::unordered_map<std::string, bool> wasDeadBefore;

AutoEZ::AutoEZ()
    : Module("AutoEZ", "Automatically says EZ when you kill someone", Category::WORLD) {}

void AutoEZ::onNormalTick(LocalPlayer* localPlayer) {
    if(!localPlayer || !localPlayer->level)
        return;

    for(auto& entity : localPlayer->level->getRuntimeActorList()) {
        if(!TargetUtil::isTargetValid(entity, false, 1000.f))
            continue;
        if(entity == localPlayer)
            continue;

        std::string name = entity->getNameTag();

        if(!entity->isAlive()) {
            if(!wasDeadBefore[name]) {
                std::shared_ptr<Packet> packet = MinecraftPacket::createPacket(PacketID::Text);
                auto* pkt = reinterpret_cast<TextPacket*>(packet.get());

                pkt->mType = TextPacketType::Chat;
                pkt->mMessage = "Yo " + name + " You're EZ buddy";
                pkt->mPlatformId = "";
                pkt->mLocalize = false;
                pkt->mXuid = "";
                pkt->mAuthor = "";

                Game.getPacketSender()->sendToServer(
                    pkt);

                wasDeadBefore[name] = true;
            }
        } else {
            wasDeadBefore[name] = false;
        }
    }
}
