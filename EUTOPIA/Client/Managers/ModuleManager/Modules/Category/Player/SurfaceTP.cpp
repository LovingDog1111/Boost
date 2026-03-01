#include "SurfaceTP.h"

#include "../../../../../Client.h"

bool active = false;
Vec3<float> goal = Vec3<float>(0, 0, 0);
float speed2344 = 1.5f;
float threshold = 1.0f;

SurfaceTP::SurfaceTP() : Module("SurfaceTP", "EnemyTP style surface teleport", Category::WORLD) {}

void SurfaceTP::onEnable() {
    auto player = Game.getLocalPlayer();
    if(!player) {
        setEnabled(false);
        return;
    }

    auto region = Game.getClientInstance()->getRegion();
    if(!region) {
        setEnabled(false);
        return;
    }

    Vec3<float> pos = player->getPos();
    int targetY = -1;

    for(int y = pos.y; y < 256; y++) {

        Block* base = region->getBlock(BlockPos(pos.x, y, pos.z));
        if(!base || !base->blockLegacy)
            continue;
        if(!base->blockLegacy->mSolid)
            continue;

        Block* a1 = region->getBlock(BlockPos(pos.x, y + 1, pos.z));
        Block* a2 = region->getBlock(BlockPos(pos.x, y + 2, pos.z));
        if(!a1 || !a2)
            continue;
        if(a1->blockLegacy->blockId != 0)
            continue;
        if(a2->blockLegacy->blockId != 0)
            continue;

        targetY = y;
        break;
    }

    if(targetY == -1) {
        Game.DisplayClientMessage("%s[Top] No spot", MCTF::RED);
        setEnabled(false);
        return;
    }

    goal = Vec3<float>(pos.x + 0.5f, targetY + 1.0f, pos.z + 0.5f);
    active = true;
}

void SurfaceTP::onNormalTick(LocalPlayer* player) {
    if(!active || !this->isEnabled() || !player)
        return;

    Vec3<float> cur = player->getPos();
    Vec3<float> delta(goal.x - cur.x, goal.y - cur.y, goal.z - cur.z);
    float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);

    if(dist <= threshold) {
        AABB a = player->getAABB(true);
        a.upper.y = a.lower.y + 1.8f;
        player->setAABB(a);
        active = false;
        Game.DisplayClientMessage("%s[Top] Reached surface", MCTF::GREEN);
        setEnabled(false);
        return;
    }

    float step = std::min(speed2344, dist);
    float ratio = step / dist;
    Vec3<float> move(delta.x * ratio, delta.y * ratio, delta.z * ratio);

    player->lerpMotion(move);

    AABB a = player->getAABB(true);
    a.lower = a.lower.add(move);
    a.upper = a.upper.add(move);
    player->setAABB(a);
}

void SurfaceTP::onDisable() {
    active = false;
}
