#pragma once

#include "..\SDK\GlobalInstance.h"
#include <vector>

class BlockUtils {
   public:
    static bool isOverVoid(const Vec3<float>& pos) {
        Vec3<float> checkPos = pos;
        for(int i = 0; i < 256; i++) {
            Vec3<float> below = checkPos + Vec3<float>(0, -i, 0);
            if(!isAirBlock(below))
                return false;
        }
        return true;
    }

    static bool isGoodBlock(const Vec3<float>& pos) {
        return !isAirBlock(pos);
    }

    static bool isAirBlock(const Vec3<float>& pos) {
        auto block = GI::getRegion()->getBlock(pos.x, pos.y, pos.z);
        if(!block)
            return true;
        return block->blockLegacy->blockId == 0;
    }
};
