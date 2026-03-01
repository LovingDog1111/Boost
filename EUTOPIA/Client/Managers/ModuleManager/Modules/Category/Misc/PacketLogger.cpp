#include "PacketLogger.h"



PacketLogger::PacketLogger() : Module("PacketLogger", "Logs incoming packets", Category::MISC) {}

void PacketLogger::onNormalTick(LocalPlayer* player) {
    

    for(auto& packet : packetQueue) {
        if(!packet)
            continue;
        std::string idStr = std::to_string(static_cast<int>(packet->getId()));
        std::string msg = "[Incoming] " + packet->getName() + " (ID: " + idStr + ")";
        GI::DisplayClientMessage(msg.c_str());
    }

    packetQueue.clear();
}