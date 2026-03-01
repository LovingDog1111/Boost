#include "Scaffold.h"

#include <cmath>

#include "../../../../../../SDK/GlobalInstance.h"
#include "../../../../../../SDK/NetWork/PacketSender.h"
#include "../../../../../../SDK/NetWork/Packets/MovePlayerPacket.h"
#include "../../../../../../SDK/World/Actor/GameMode.h"
#include "../../../../../../SDK/World/Actor/LocalPlayer.h"
#include "../../../../../../SDK/World/Level/BlockSource.h"
#include "../../../../../../Utils/Maths.h"
#include "../../../../../../Utils/Minecraft/InvUtil.h"
#include "../../../../../../Utils/Minecraft/WorldUtil.h"

Scaffold::Scaffold() : Module("Scaffold", "Places blocks under you", Category::WORLD) {
    registerSetting(
        new BoolSetting("Tower Mode", "Enable tower building (hold space)", &towerMode, false));
    registerSetting(
        new BoolSetting("Air Place", "Allow block placement while jumping", &airPlace, false));
}

void Scaffold::onNormalTick(LocalPlayer* localPlayer) {
    if(!localPlayer)
        return;

    Vec3<float> velocity = localPlayer->getStateVectorComponent()->mVelocity;
    static float lastVelocityY = 0.0f;
    bool isJumpingByVelocity = (velocity.y > lastVelocityY + 0.1f && velocity.y > 0.1f);
    lastVelocityY = velocity.y;

    if(towerMode) {
        auto moveInput = localPlayer->getMoveInputComponent();
        BlockPos below = localPlayer->getPos().floor().CastTo<int>();
        below = BlockPos(below.x, below.y - 1, below.z);

        if(below.y > 319) {
            updateHUDAnimation(0.016f);
            return;
        }
        bool shouldPlace = false;
        if(moveInput && moveInput->mIsJumping) {
            shouldPlace = true;
        } else if(isJumpingByVelocity) {
            shouldPlace = true;
        }

        else if(!localPlayer->onGround() && !WorldUtil::isValidPlacePos(below)) {
            shouldPlace = true;
        }

        if(shouldPlace && (WorldUtil::isValidPlacePos(below) || airPlace)) {
            int slot = selectBlockSlot(localPlayer);
            if(slot != -1) {
                localPlayer->getsupplies()->mSelectedSlot = slot;
                InvUtil::sendMobEquipment(slot);
                WorldUtil::placeBlock(below, -1);
                if(moveInput && moveInput->mIsJumping) {
                    localPlayer->jumpFromGround();
                }
            }
        }
        updateHUDAnimation(0.016f);
        return;
    }

    AABB aabb = localPlayer->getAABBShapeComponent()->getAABB();
    Vec3<float> feetPos = aabb.getCenter();
    feetPos.y = aabb.lower.y;
    BlockPos placePos = feetPos.add(Vec3<float>(0.f, 0.5f, 0.f)).floor().CastTo<int>();
    ;
    placePos = BlockPos(placePos.x, placePos.y - 1, placePos.z);

    if(placePos.y > 319) {
        return;
    }

    bool canPlace = WorldUtil::isValidPlacePos(placePos);

    if(airPlace && !canPlace) {
        canPlace = true;
    }

    Vec3<float> playerVel = localPlayer->getStateVectorComponent()->mVelocity;
    bool isMoving = (abs(playerVel.x) > 0.1f || abs(playerVel.z) > 0.1f);

    if(!canPlace && isMoving) {
        Vec3<float> nextPos = localPlayer->getPos().add(playerVel.mul(0.2f));
        BlockPos nextPlacePos = nextPos.floor().CastTo<int>();
        nextPlacePos = BlockPos(nextPlacePos.x, nextPlacePos.y - 1, nextPlacePos.z);

        if(nextPlacePos.y <= 319 && (WorldUtil::isValidPlacePos(nextPlacePos) || airPlace)) {
            placePos = nextPlacePos;
            canPlace = true;
        }
    }

    if(!canPlace)
        return;

    int slot = selectBlockSlot(localPlayer);
    if(slot == -1)
        return;
    int lastSlot = localPlayer->getsupplies()->mSelectedSlot;
    if(lastSlot != slot) {
        localPlayer->getsupplies()->mSelectedSlot = slot;
        InvUtil::sendMobEquipment(slot);
    }

    WorldUtil::placeBlock(placePos, -1);
    updateHUDAnimation(0.016f);
}

BlockPos Scaffold::getPlacePos(LocalPlayer* player) {
    Vec3<float> pos = player->getPos();

    static const int dx[] = {0, 1, -1, 0, 0};
    static const int dz[] = {0, 0, 0, 1, -1};
    for(int i = 0; i < 5; ++i) {
        BlockPos candidate(static_cast<int>(std::floor(pos.x + dx[i])),
                           static_cast<int>(std::floor(pos.y)) - 1,
                           static_cast<int>(std::floor(pos.z + dz[i])));
        if(canPlaceBlock(candidate) && candidate.y <= 319)
            return candidate;
    }
    return BlockPos(INT_MAX, INT_MAX, INT_MAX);
}

bool Scaffold::canPlaceBlock(const BlockPos& pos) {
    auto region = GI::getRegion();
    if(!region)
        return false;
    Block* block = region->getBlock(pos);
    if(!block || !block->blockLegcy)
        return false;
    return block->blockLegcy->isAir();
}

int Scaffold::selectBlockSlot(LocalPlayer* player) {
    auto supplies = player->getsupplies();
    if(!supplies || !supplies->container)
        return -1;
    for(int i = 0; i < 9; ++i) {
        ItemStack* stack = supplies->container->getItem(i);
        if(InvUtil::isVaildItem(stack) && stack->isBlock()) {
            return i;
        }
    }
    return -1;
}

void Scaffold::updateHUDAnimation(float deltaTime) {
    int target = 0;
    mAnimBlockCount += (target - mAnimBlockCount) * deltaTime * 10.f;
}