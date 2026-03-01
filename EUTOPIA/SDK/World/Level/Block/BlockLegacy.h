#pragma once
#include "../../Utils/MemoryUtil.h"
#include "../../Utils/Maths.h"
class Block;
class BlockSource;
enum class MaterialType : int {
    Air = 0,
    Dirt = 1,
    Wood = 2,
    Metal = 3,
    Grate = 4,
    Water = 5,
    Lava = 6,
    Leaves = 7,
    Plant = 8,
    SolidPlant = 9,
    Fire = 10,
    Glass = 11,
    Explosive = 12,
    Ice = 13,
    PowderSnow = 14,
    Cactus = 15,
    Portal = 16,
    StoneDecoration = 17,
    Bubble = 18,
    Barrier = 19,
    DecorationSolid = 20,
    ClientRequestPlaceholder = 21,
    StructureVoid = 22,
    Solid = 23,
    NonSolid = 24,
    Any = 25,
};

class Material {
   public:
    CLASS_MEMBER(MaterialType, mType, 0x0);
    CLASS_MEMBER(bool, mIsFlammable, 0x4);
    CLASS_MEMBER(bool, mIsNeverBuildable, 0x5);
    CLASS_MEMBER(bool, mIsLiquid, 0x6);
    CLASS_MEMBER(bool, mIsBlockingMotion, 0xC);
    CLASS_MEMBER(bool, mIsBlockingPrecipitation, 0xD);
    CLASS_MEMBER(bool, mIsSolid, 0xE);
    CLASS_MEMBER(bool, mIsSuperHot, 0xF);

    bool isTopSolid(bool includeWater, bool includeLeaves) {
        if(mType == MaterialType::Leaves)
            return includeLeaves;
        if(!includeWater)
            return mIsBlockingMotion;
        if(mType != MaterialType::Water)
            return mIsBlockingMotion;
        return true;
    }
};

class BlockLegacy {
public:
    CLASS_MEMBER(std::string, mtranslateName, 0x8);
 CLASS_MEMBER(std::string, mName, 0xA0); 
 CLASS_MEMBER(std::string, BlockName, 0xA0);
 CLASS_MEMBER(std::string, blockName, 0xA0);
 CLASS_MEMBER(std::string, mNameSpace, 0xC0); 
	CLASS_MEMBER(uint16_t, blockid, 0x1D6);
 CLASS_MEMBER(uint16_t, blockId, 0x1D6);
 CLASS_MEMBER(Material*, mMaterial, 0x160);
    CLASS_MEMBER(bool, mSolid, 0x19C);
	
public:
    bool canBeBuiltOver(BlockSource* blockSource, BlockPos const &pos) {
		return !mSolid;
	}
	AABB* getOutline(Block* block, BlockSource* blockSource, BlockPos* blockPos, AABB* aabbOut) {
		return MemoryUtil::callVirtualFunc<AABB*>(9, this, block, blockSource, blockPos, aabbOut);
	}
  bool isAir()
{
    return blockid == 0;
}

  	Material* getMaterial() {
    return mMaterial;
}

};