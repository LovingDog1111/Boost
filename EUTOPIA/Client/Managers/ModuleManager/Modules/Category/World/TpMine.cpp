#define NOMINMAX
#include "TpMine.h"

#include <algorithm>
#include <cfloat>
#include <cmath>

#include "../../../ModuleManager.h"
#include "../../SDK/GlobalInstance.h"

TpMine::TpMine() : Module("TpMine", "Teleport to selected ore", Category::WORLD) {
    registerSetting(new BoolSetting("Diamond", "Mine Diamond", &mineDiamond, true));
    registerSetting(new BoolSetting("Iron", "Mine Iron", &mineIron, false));
    registerSetting(new BoolSetting("Gold", "Mine Gold", &mineGold, false));
    registerSetting(new BoolSetting("Coal", "Mine Coal", &mineCoal, false));
    registerSetting(new BoolSetting("Lapis", "Mine Lapis", &mineLapis, false));
    registerSetting(new BoolSetting("Redstone", "Mine Redstone", &mineRedstone, false));
    registerSetting(new BoolSetting("Emerald", "Mine Emerald", &mineEmerald, false));
    registerSetting(new BoolSetting("Quartz", "Mine Quartz", &mineQuartz, false));
    registerSetting(new BoolSetting("AncientDebris", "Mine Ancient Debris", &mineAncient, false));
    registerSetting(new SliderSetting<int>("Range", "Scan range", &range, 80, 10, 150));
}

void TpMine::onEnable() {
    hasTarget = false;

    LocalPlayer* player = GI::getLocalPlayer();
    if(!player) {
        setEnabled(false);
        return;
    }

    BlockSource* region = GI::getClientInstance()->getRegion();
    if(!region) {
        setEnabled(false);
        return;
    }

    std::vector<int> ores;
    if(mineDiamond)
        ores.push_back(56);
    if(mineIron)
        ores.push_back(15);
    if(mineGold)
        ores.push_back(14);
    if(mineCoal)
        ores.push_back(16);
    if(mineQuartz)
        ores.push_back(153);
    if(mineLapis)
        ores.push_back(21);
    if(mineRedstone)
        ores.push_back(74);
    if(mineEmerald)
        ores.push_back(129);
    if(mineAncient)
        ores.push_back(526);

    if(ores.empty()) {
        setEnabled(false);
        return;
    }

    Vec3<float> pos = player->getPos();
    BlockPos base((int)pos.x, (int)pos.y, (int)pos.z);

    float closestDist = FLT_MAX;
    BlockPos closest(0, 0, 0);

    for(int x = -range; x <= range; x++) {
        for(int y = -3; y <= 3; y++) {
            for(int z = -range; z <= range; z++) {
                BlockPos bp = base.add2(x, y, z);
                Block* blk = region->getBlock(bp);
                if(!blk || !blk->blockLegacy)
                    continue;
                int id = blk->blockLegacy->blockId;
                if(std::find(ores.begin(), ores.end(), id) == ores.end())
                    continue;

                float dist =
                    std::sqrt((pos.x - bp.x) * (pos.x - bp.x) + (pos.y - bp.y) * (pos.y - bp.y) +
                              (pos.z - bp.z) * (pos.z - bp.z));

                if(dist < closestDist) {
                    closestDist = dist;
                    closest = bp;
                }
            }
        }
    }

    if(closestDist == FLT_MAX) {
        setEnabled(false);
        return;
    }

    targetPos = closest.toFloat().add2(0.5f, 0.5f, 0.5f);
    hasTarget = true;
}

void TpMine::onNormalTick(LocalPlayer* player) {
    if(!hasTarget || !player)
        return;

    Vec3<float> pos = player->getPos();
    Vec3<float> delta = targetPos.sub(pos);
    float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);

    if(dist < 1.0f) {
        hasTarget = false;
        setEnabled(false);
        return;
    }

    float step = std::min(1.5f, dist);
    Vec3<float> movement = delta * (step / dist);
    Utils::setTimer(100.f);
    player->lerpMotion(movement);
    AABB a = player->getAABB(true);
    a.lower = a.lower.add(movement);
    a.upper = a.upper.add(movement);
    player->setAABB(a);
    Utils::resetTimer();
}