
#pragma once
#include "../../Utils/Maths.h"
#include "Packet.h"

class SetActorMotionPacket : public Packet {
   public:
    static const PacketID ID = PacketID::SetActorMotion;
    int64_t mRuntimeID;
   Vec3<float> mMotion;

    SetActorMotionPacket() = default;
};