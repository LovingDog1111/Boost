#pragma once

#include "Packet.h"

class RemoveActorPacket : public Packet {
   public:
    static const PacketID ID = PacketID::RemoveActor;

    int64_t mRuntimeID;
};