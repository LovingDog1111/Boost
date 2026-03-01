#include "Nuker.h"

Nuker::Nuker() : Module("Nuker", "Break multiple blocks at once.", Category::WORLD) {
    registerSetting(new SliderSetting<int>("Radius", "The Size", &nukerRadius, nukerRadius, 1, 10));
}

Nuker::~Nuker() {}

void Nuker::onNormalTick(LocalPlayer* gm) {
    Vec3<int> tempPos = gm->getPos().toInt();
    for(int x = -nukerRadius; x < nukerRadius; x++) {
        for(int y = -nukerRadius; y < nukerRadius; y++) {
            for(int z = -nukerRadius; z < nukerRadius; z++) {
                Vec3<int> checkPos = tempPos + Vec3<int>(x, y, z);
                if(checkPos.y > -64) {
                    Block* block = Game.getClientInstance()->getRegion()->getBlock(checkPos);
                    if(block && block->blockLegacy) {     
                        if(block->blockLegacy->mSolid) {  
                            gm->gamemode->destroyBlock(checkPos, 1);
                        }
                    }
                }
            }
        }
    }
}
