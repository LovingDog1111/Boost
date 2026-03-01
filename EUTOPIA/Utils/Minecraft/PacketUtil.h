#pragma once
#include <memory>
#include "../../SDK/NetWork/Packets/MobEquipmentPacket.h"

class PacketUtil {
   public:
    static void spoofSlot(int slot, bool bypassHook = true);
    static void cancelNetworkPacket(Packet* packet);
};