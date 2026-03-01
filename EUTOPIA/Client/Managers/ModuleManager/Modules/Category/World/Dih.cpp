#include "Dih.h"

#include "../../../../../../SDK/GlobalInstance.h"
#include "../../../../../../SDK/World/Actor/GameMode.h"
#include "../../../../../../SDK/World/Actor/LocalPlayer.h"
#include "../../../../../../SDK/World/Level/BlockSource.h"
#include "../../../../../../Utils/Maths.h"
#include "../../../../../../Utils/Minecraft/InvUtil.h"
#include "../../../../../../Utils/Minecraft/WorldUtil.h"

Dih::Dih() : Module("Dih", "Dihdihdihdih", Category::WORLD) {
    registerSetting(new BoolSetting("Tower Mode", "", &towerMode, false));
    registerSetting(new BoolSetting("Air Place", "", &airPlace, false));

}

void Dih::onNormalTick(LocalPlayer* localPlayer) {
    if(!localPlayer)
        return;

    buildTimer += 0.016f;
    if(buildTimer < 5.f)
        return;
    buildTimer = 0.f;

    Vec3<float> pos = localPlayer->getPos();
    Vec3<float> dir = pos.sub(lastPos);
    float dirLength = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    if(dirLength < 0.001f)
        dir = Vec3<float>(0.f, 0.f, 1.f);
    lastPos = pos;

    int dx = (std::fabs(dir.x) > std::fabs(dir.z)) ? (dir.x > 0 ? 1 : -1) : 0;
    int dz = (dx == 0) ? (dir.z > 0 ? 1 : -1) : 0;

    BlockPos under(static_cast<int>(std::floor(pos.x)), static_cast<int>(std::floor(pos.y)) - 1,
                   static_cast<int>(std::floor(pos.z)));

    BlockPos vertical(under.x, under.y, under.z);
    BlockPos horizontal[3];                        
    horizontal[0] = BlockPos(under.x - 1, under.y, under.z);
    horizontal[1] = BlockPos(under.x, under.y, under.z);
    horizontal[2] = BlockPos(under.x + 1, under.y, under.z);

    BlockPos positions[4] = {vertical, horizontal[0], horizontal[1], horizontal[2]};

    for(int i = 0; i < 4; ++i) {
        BlockPos p = positions[i];
        if(p.y > 319)
            continue;
        if(!canPlaceBlock(p) && !airPlace)
            continue;
        int slot = selectBlockSlot(localPlayer);
        if(slot == -1)
            continue;
        if(localPlayer->getsupplies()->mSelectedSlot != slot) {
            localPlayer->getsupplies()->mSelectedSlot = slot;
            InvUtil::sendMobEquipment(slot);
        }
        WorldUtil::placeBlock(p, -1);
    }

    if(towerMode && localPlayer->getMoveInputComponent() &&
       localPlayer->getMoveInputComponent()->mIsJumping) {
        localPlayer->jumpFromGround();
    }

    updateHUDAnimation(0.016f);
}

bool Dih::canPlaceBlock(const BlockPos& pos) {
    auto region = GI::getRegion();
    if(!region)
        return false;
    Block* block = region->getBlock(pos);
    if(!block || !block->blockLegcy)
        return false;
    return block->blockLegcy->isAir();
}

int Dih::selectBlockSlot(LocalPlayer* player) {
    auto supplies = player->getsupplies();
    if(!supplies || !supplies->container)
        return -1;
    for(int i = 0; i < 9; ++i) {
        ItemStack* stack = supplies->container->getItem(i);
        if(InvUtil::isVaildItem(stack) && stack->isBlock())
            return i;
    }
    return -1;
}

void Dih::updateHUDAnimation(float deltaTime) {
    int target = 0;
    mAnimBlockCount += (target - mAnimBlockCount) * deltaTime * 10.f;
}
