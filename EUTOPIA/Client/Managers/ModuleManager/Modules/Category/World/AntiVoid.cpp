#include "AntiVoid.h"

#include <algorithm>
#include <cmath>

#include "..\\Utils\\Minecraft\\BlockUtils.h"

AntiVoid::AntiVoid() : Module("AntiVoid", "Prevent falling into the void", Category::WORLD) {
    registerSetting(new BoolSetting("Teleport", "Teleport back when falling", &mTeleport, true));
}

void AntiVoid::onEnable() {
    auto player = Game.getLocalPlayer();
    if(!player)
        return;

    mOnGroundPositions.clear();
    mTeleported = false;
    mCanTeleport = true;
}

void AntiVoid::onNormalTick(LocalPlayer* player) {
    if(!player)
        return;

    if(!player->level)
        return;

    Vec3<float> playerPos = player->getPos();

    if(player->onGround()) {
        mCanTeleport = true;
        mTeleported = false;

        if(mTeleport) {
            mOnGroundPositions.push_back(playerPos);
            if(mOnGroundPositions.size() > 100)
                mOnGroundPositions.erase(mOnGroundPositions.begin());
        }
        return;
    }

    bool hasTeleported = mTeleported && mTpOnce;
    Vec3<float> checkPos(playerPos.x, playerPos.y - 1.8f, playerPos.z);

    if(player->getFallDisntance() > mFallDistance && (!hasTeleported || !mTpOnce) && mCanTeleport &&
       BlockUtils::isOverVoid(checkPos)) {
        if(!mTeleport) {
            player->setFallDisntance(0.0f);
            mOnGroundPositions.clear();
            if(mTpOnce)
                mCanTeleport = false;
            mTeleported = true;
            return;
        }

        if(mOnGroundPositions.empty())
            return;

        Vec3<float> bestPos(0.f, 0.f, 0.f);
        bool found = false;

        for(auto it = mOnGroundPositions.rbegin(); it != mOnGroundPositions.rend(); ++it) {
            Vec3<float> blockPos(it->x, it->y - 2.0f, it->z);
            if(BlockUtils::isGoodBlock(blockPos)) {
                bestPos = blockPos;
                found = true;
                break;
            }
        }

        if(!found)
            return;

        Vec3<float> targetPos(bestPos.x, bestPos.y + 2.0f, bestPos.z);
        Vec3<float> delta(targetPos.x - playerPos.x, targetPos.y - playerPos.y,
                          targetPos.z - playerPos.z);

        float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
        if(dist <= 0.0001f || std::isnan(dist) || std::isinf(dist))
            return;

        float step = std::min(1.5f, dist);
        float scale = step / dist;

        Vec3<float> movement(delta.x * scale, delta.y * scale, delta.z * scale);

        player->lerpMotion(movement);

        AABB a = player->getAABB(true);
        a.lower = a.lower.add(movement);
        a.upper = a.upper.add(movement);
        player->setAABB(a);

        player->setFallDisntance(0.0f);
        mTeleported = true;

        Game.DisplayClientMessage("[AntiVoid] Teleported to safe position!");

        if(mTpOnce) {
            mOnGroundPositions.clear();
            mCanTeleport = false;
        }
    }
}