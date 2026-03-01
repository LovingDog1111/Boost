#pragma once

#include "Packet.h"
#include <Maths.h>
#include "../../World/Level/Block/Block.h"
enum class BlockUpdateFlag : unsigned char {
    None = 0,
    Neighbors = 1 << 0,
    Network = 1 << 1,
    NoGraphic = 1 << 2,
    Priority = 1 << 3,
    ForceNoticeListener = 1 << 4,
    All = Neighbors | Network,  // default value in BDS
    AllPriority = All | Priority,
};

class UpdateBlockPacket : public Packet {
   public:
    /*    BlockPos Pos;
        uint32_t Layer;
        uint8_t UpdateFlags;
        uint64_t BlockRuntimeId;*/
    static const PacketID ID = PacketID::UpdateBlock;

    enum class BlockLayer : unsigned int {
        Standard = 0,
        Extra = 1,
        Count = 2,
    };

   Vec3<float> mPos;               // this+0x30
    BlockLayer mLayer;             // this+0x3C
    BlockUpdateFlag mUpdateFlags;  // this+0x40
    unsigned int mBlockRuntimeId;  // this+0x44
};

class UpdateBlockSyncedPacket : public ::UpdateBlockPacket {
   public:
    ActorBlockSyncMessage mEntityBlockSyncMessage;  // this+0x48
};
