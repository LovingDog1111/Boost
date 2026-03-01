#pragma once
#include "Block/Block.h"
#include "../../../Utils/Maths.h"
#include "../../../Utils/MemoryUtil.h"
#include "ChunkPos.h"
#include "Dimension.h"
class ChunkBorders;
class BlockSource {
   public:
    CLASS_MEMBER(Dimension*, dimension, 0x30);

   public:
    //Block* getBlock(const BlockPos& pos) {
    //    return MemoryUtil::callVirtualFunc<Block*>(2, this, pos);
    //}
    virtual ~BlockSource() = 0;                                             // 0x0
    virtual Block* getBlock(int, int, int) const = 0;                  // 0x1
    virtual Block* getBlock(BlockPos const&) const = 0;                // 0x2
    virtual Block* getBlock(BlockPos const&, unsigned int) const = 0;  // 0x3
    virtual void* getBlockEntity(BlockPos const&) const = 0;                // 0x4
    virtual void getExtraBlock(BlockPos const&) const = 0;                  // 0x5
    virtual void getLiquidBlock(BlockPos const&) const = 0;                 // 0x6
    virtual bool hasBlock(BlockPos const&) const = 0;                       // 0x7
    virtual void containsAnyLiquid(AABB const&) const = 0;                  // 0x8
    virtual void containsMaterial(AABB const&, int matType) const = 0;      // 0x9
    virtual void getMaterial(BlockPos const&) const = 0;                    // 0xA
    virtual void getMaterial(int, int, int) const = 0;                      // 0xB
    virtual bool hasChunksAt(ChunkBorders const&, bool) const = 0;                // 0xC
    virtual bool hasChunksAt(BlockPos const&, int, bool) const = 0;         // 0xD
    virtual bool hasChunksAt(AABB const&, bool) const = 0;                  // 0xE
    virtual void getDimensionId() const = 0;                                // 0xF
    virtual void fetchAABBs(std::vector<AABB, std::allocator<AABB>>&, AABB const&,
                            bool) const = 0;  // 0x10
    virtual void fetchCollisionShapes(
        std::vector<AABB, std::allocator<AABB>>&, AABB const&, float*, bool,
        void* /*optional_ref<GetCollisionShapeInterface const>*/) const = 0;  // 0x11
    virtual void getTallestCollisionShape(
        AABB const&, float*, bool,
        /*optional_ref<GetCollisionShapeInterface const>*/ void*) const = 0;      // 0x12
    virtual void getBrightness(BlockPos const&) const = 0;                        // 0x13
    virtual void getWeakRef() = 0;                                                // 0x14
    virtual void addListener(/*BlockSourceListener&*/ void*) = 0;                 // 0x15
    virtual void removeListener(/*BlockSourceListener&*/ void*) = 0;              // 0x16
    virtual void fetchEntities(class Actor const*, AABB const&, bool, bool) = 0;  // 0x17
    virtual void fetchEntities(int actorType, AABB const&, class Actor const*,
                               std::function<bool(Actor*)>) = 0;  // 0x18
    virtual void setBlock(BlockPos const&, class Block const&, int,
                          class ActorBlockSyncMessage const*, class Actor*) = 0;  // 0x19
    virtual void getMaxHeight() const = 0;                                        // 0x1A
    virtual void getMinHeight() const = 0;                                        // 0x1B
    virtual void getDimension() const = 0;                                        // 0x1C
    virtual void getDimensionConst() const = 0;                                   // 0x1D
    virtual void getDimension() = 0;                                              // 0x1E
    virtual void* getChunkAt(BlockPos const&) const = 0;                          // 0x1F
    virtual void getILevel() const = 0;                                           // 0x20
    virtual void fetchAABBs(AABB const&, bool) = 0;                               // 0x21
    virtual void fetchCollisionShapes(AABB const&, float*, bool,
                                      class IActorMovementProxy*) = 0;  // 0x22
    virtual void getChunkSource() = 0;                                  // 0x23
    virtual bool isSolidBlockingBlock(BlockPos const&) const = 0;       // 0x24
    virtual bool isSolidBlockingBlock(int, int, int) const = 0;         // 0x25
    virtual void areChunksFullyLoaded(BlockPos const&, int) const = 0;  // 0x26
    virtual bool canDoBlockDrops() const = 0;                           // 0x27
    virtual bool canDoContainedItemDrops() const = 0;                   // 0x28

   public:
    float getSeenPercent(const Vec3<float>& pos, const AABB& aabb) {
        static auto addr = MemoryUtil::findSignature(
            "40 55 56 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 "
            "24");
        using func_t = float(__thiscall*)(BlockSource*, const Vec3<float>&, const AABB&);
        static func_t Func = reinterpret_cast<func_t>(addr);
        return Func(this, pos, aabb);
    }
};