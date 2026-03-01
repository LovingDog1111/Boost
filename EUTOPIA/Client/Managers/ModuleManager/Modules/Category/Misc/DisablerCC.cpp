#include "DisablerCC.h"

#include <../SDK/NetWork/Packets/NetworkStackLatencyPacket.h>
#include <../SDK/NetWork/Packets/PlayerAuthInputPacket.h>
#include "../../Utils/DrawUtil.h"
#include "../Client/Managers/ModuleManager/ModuleManager.h"
#include "../Client/Managers/ModuleManager/Modules/Category/Client/ColorsMod.h"
#include <Windows.h>

DisablerCC::DisablerCC() : Module("DisablerCC", "Disable the anticheat", Category::MISC) {
    registerSetting(new SliderSetting<float>("Target ping", "Desired ping (ms)", &targetPing, 100.f,
                                             100.f, 6000.f));
    registerSetting(new BoolSetting("MSpoof", "Send double movement packets (may cause lagback)",
                                    &movementSpoof, false));

}

void DisablerCC::onEnable() {
    mPacketQueue.clear();
    GI::DisplayClientMessage(
        ("[DisablerCC] Target ping set to: " + std::to_string(targetPing)).c_str());
}

void DisablerCC::onRecievePacket(Packet* packet, bool* cancel) {
    if(!packet)
        return;
    if(packet->getId() != PacketID::NetworkStackLatency)
        return;

    auto* latency = static_cast<NetworkStackLatencyPacket*>(packet);
    if(!latency || !latency->mFromServer)
        return;

    auto sender = GI::getPacketSender();
    if(!sender)
        return;

    uint64_t now = GetTickCount64();
    uint64_t targetSendTime = now + static_cast<uint64_t>(targetPing);

    QueuedLatencyPacket queuedPacket;
    queuedPacket.originalTimestamp = latency->mCreateTime;
    queuedPacket.sendTime = targetSendTime;
    mPacketQueue.push_back(queuedPacket);
    
    /*
    latency->mFromServer = false;
    latency->mCreateTime = 0;
    */
    PacketUtil::cancelNetworkPacket(latency);
}

void DisablerCC::processLatencyQueue() {
    if(mPacketQueue.empty())
        return;

    auto sender = GI::getPacketSender();
    if(!sender)
        return;

    uint64_t now = GetTickCount64();
    std::vector<QueuedLatencyPacket> remainingPackets;

    for(const auto& queuedPacket : mPacketQueue) {
        if(now >= queuedPacket.sendTime) {
            auto responsePacket = MinecraftPacket::createPacket<NetworkStackLatencyPacket>();
            if(!responsePacket)
                continue;
            responsePacket->mCreateTime = queuedPacket.originalTimestamp;
            responsePacket->mFromServer = false;
            sender->sendToServer(responsePacket.get());
        } else {
            remainingPackets.push_back(queuedPacket);
        }
    }

    mPacketQueue = std::move(remainingPackets);
}

void DisablerCC::onNormalTick(LocalPlayer* player) {
    processLatencyQueue();
}

void DisablerCC::onSendPacket(Packet* packet) {
    if(!packet)
        return;
    if(movementSpoof && packet->getId() == PacketID::PlayerAuthInput) {
        auto* movePacket = static_cast<PlayerAuthInputPacket*>(packet);
        auto sender = GI::getPacketSender();
        if(sender) {
            sender->sendToServer(movePacket);
            sender->sendToServer(movePacket);
        }
    }
}

void DisablerCC::onDisable() {
    auto sender = GI::getPacketSender();
    if(sender) {
        for(const auto& queuedPacket : mPacketQueue) {
            auto responsePacket = MinecraftPacket::createPacket<NetworkStackLatencyPacket>();
            if(!responsePacket)
                continue;
            responsePacket->mCreateTime = queuedPacket.originalTimestamp;
            responsePacket->mFromServer = false;
            sender->sendToServer(responsePacket.get());
        }
    }
    mPacketQueue.clear();
}

void DisablerCC::onD2DRender() {

}

void DisablerCC::onMCRender(MinecraftUIRenderContext* ctx) {

}