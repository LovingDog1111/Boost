#include "AutoEZ.h"

#include <unordered_map>

#include "../SDK/NetWork/Packets/TextPacket.h"
#include "../Utils/Minecraft/TargetUtil.h"

std::unordered_map<std::string, bool> wasDeadBefore;

AutoEZ::AutoEZ()
    : Module("AutoEZ", "Automatically says EZ when you kill someone", Category::WORLD) {}

void AutoEZ::onNormalTick(LocalPlayer* localPlayer) {
    if(!localPlayer)
        return;

    if(!localPlayer->level)
        return;

    auto* sender = Game.getPacketSender();
    if(!sender)
        return;

    const auto& actorList = localPlayer->level->getRuntimeActorList();

    for(auto* entity : actorList) {
        if(!entity)
            continue;

        if(entity == localPlayer)
            continue;

        if(!TargetUtil::isTargetValid(entity, false, 1000.f))
            continue;

        std::string name = entity->getNameTag();
        if(name.empty())
            continue;

        bool isAlive = entity->isAlive();

        if(!isAlive) {
            if(!wasDeadBefore[name]) {
                std::shared_ptr<Packet> packet = MinecraftPacket::createPacket(PacketID::Text);
                if(!packet)
                    continue;

                auto* pkt = reinterpret_cast<TextPacket*>(packet.get());
                if(!pkt)
                    continue;

                pkt->mType = TextPacketType::Chat;
                pkt->mMessage = "Yo " + name + " You're EZ buddy";
                pkt->mPlatformId = "";
                pkt->mLocalize = false;
                pkt->mXuid = "";
                pkt->mAuthor = "";

                sender->sendToServer(pkt);

                wasDeadBefore[name] = true;
            }
        } else {
            wasDeadBefore[name] = false;
        }
    }
}