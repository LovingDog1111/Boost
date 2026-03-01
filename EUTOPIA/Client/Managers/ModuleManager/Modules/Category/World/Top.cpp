#define NOMINMAX
#include "Top.h"

#include <cfloat>
#include <cmath>

Top::Top() : Module("Top", "Teleport to surface", Category::WORLD) {}

void Top::onEnable() {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer) {
        this->setEnabled(false);
        return;
    }

    BlockSource* region = GI::getClientInstance()->getRegion();
    if(!region) {
        this->setEnabled(false);
        return;
    }

    Vec3<float> pos = localPlayer->getPos();
    int x = (int)pos.x;
    int z = (int)pos.z;

    for(int y = 319; y >= 1; --y) {
        Block* ground = region->getBlock(BlockPos(x, y, z));
        Block* air1 = region->getBlock(BlockPos(x, y + 1, z));
        Block* air2 = region->getBlock(BlockPos(x, y + 2, z));

        if(!ground || !ground->blockLegacy || !air1 || !air1->blockLegacy || !air2 ||
           !air2->blockLegacy)
            continue;

        if(ground->blockLegacy->blockId != 0 && air1->blockLegacy->blockId == 0 &&
           air2->blockLegacy->blockId == 0) {
            startPos = pos;
            goal = Vec3<float>(x + 0.5f, y + 1.0f, z + 0.5f);
            works = true;
            break;
        }
    }

    if(!works)
        this->setEnabled(false);
}

void Top::onNormalTick(LocalPlayer* localPlayer) {
    if(!works || !localPlayer)
        return;

    Vec3<float> cur = localPlayer->getPos();
    float d = cur.dist(goal);

    if(d <= 1.2f) {
        localPlayer->teleportTo(goal, false, 0, 0, false);
        this->setEnabled(false);
        return;
    }

    Vec3<float> next = cur.lerpTo(goal, 0.25f);
    localPlayer->teleportTo(next, false, 0, 0, false);

    if(++ticks > 100) {
        this->setEnabled(false);
        return;
    }
}

void Top::onDisable() {
    works = false;
    ticks = 0;
}
