#pragma once
#include <memory>
#include <string>
#include <vector>

#include "../../ModuleBase/Module.h"

class Packet;

class PacketLogger : public Module {
   public:
    PacketLogger();
    void onNormalTick(LocalPlayer* player) override;
    std::vector<std::shared_ptr<Packet>> packetQueue;
};
