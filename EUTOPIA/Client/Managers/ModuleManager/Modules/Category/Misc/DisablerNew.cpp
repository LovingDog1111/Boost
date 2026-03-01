#include "DisablerNew.h"

#include <../SDK/NetWork/Packets/NetworkStackLatencyPacket.h>
#include <Windows.h>

#include <chrono>
#include <cstdint>
#include <unordered_map>

#define NOW                                                  \
    std::chrono::duration_cast<std::chrono::milliseconds>(   \
        std::chrono::steady_clock::now().time_since_epoch()) \
        .count()

std::unordered_map<uint64_t, uint64_t> mPacketQueue;
int Mode = 0;

DisablerNew::DisablerNew() : Module("DisablerNew", "Disable the anticheat", Category::MISC) {
    registerSetting(new EnumSetting("Mode", "", {"Sentinel"}, &Mode, 0));
}

void DisablerNew::onReceivePacket(Packet* packet, bool* cancel) {

    if(Mode == 0 && packet->getId() == PacketID::NetworkStackLatency) {
        auto latency = reinterpret_cast<NetworkStackLatencyPacket*>(packet);
        if(latency) {
            GI::DisplayClientMessage("[DEBUG] Received latency packet");


            std::string fromServerStr = std::to_string(latency->mFromServer);
            std::string createTimeStr = std::to_string(latency->mCreateTime);

            GI::DisplayClientMessage(("[DEBUG] Latency from server: " + fromServerStr).c_str());
            GI::DisplayClientMessage(("[DEBUG] CreateTime: " + createTimeStr).c_str());
        }





        if(!latency->mFromServer) {
            latency->mFromServer = true;
            return;  
        }

  
        if(mPacketQueue.size() == 150) {
            for(auto& [first, second] : mPacketQueue) {
                auto pkt = MinecraftPacket::createPacket<NetworkStackLatencyPacket>();
                pkt->mCreateTime = second;
                pkt->mFromServer = false;
                GI::getPacketSender()->sendToServer(pkt.get());
            }
            mPacketQueue.clear();
        }

        mPacketQueue[NOW] = latency->mCreateTime;

        return;  
    }
    return;
}
