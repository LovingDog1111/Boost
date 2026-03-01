#include "DashAura.h"

#include <Minecraft/TargetUtil.h>
#include <Minecraft/WorldUtil.h>

#include <algorithm>
#include <cmath>
#include <random>

bool mobs2 = false;

#include "../../../ModuleManager.h"

DashAura::DashAura() : Module("DashAura", "Auto attacks targets and returns", Category::COMBAT) {
    registerSetting(new SliderSetting<float>("Reach", "Attack reach", &range, 5.f, 1.f, 16.f));
    registerSetting(new BoolSetting("Mobs", "Attack mobs", &mobs2, false));
    registerSetting(new SliderSetting<int>("MinCPS", "Min CPS", &minCPS, 5, 0, 25));
    registerSetting(new SliderSetting<int>("MaxCPS", "Max CPS", &maxCPS, 10, 0, 25));
    registerSetting(new SliderSetting<int>("TP Delay", "Teleport delay (ticks)", &delay, 0, 0, 20));
    registerSetting(
        new SliderSetting<int>("Attack Delay", "Attack delay (ticks)", &attackDelay, 0, 0, 20));
    registerSetting(new SliderSetting<float>("TP Distance", "Base teleport distance", &tpDistance,
                                             1.f, 0.1f, 10.f));
    registerSetting(new SliderSetting<float>("TP Move Speed", "Teleport movement speed",
                                             &tpMoveSpeed, 1.f, 0.1f, 5.f));
    registerSetting(new SliderSetting<float>("Y Offset", "Vertical teleport offset", &tpYOffset,
                                             0.f, -3.f, 3.f));
    registerSetting(new BoolSetting("Through Walls", "Attack through walls", &throughWallz, false));
}

void DashAura::onEnable() {
    targetList.clear();
    lastAttackTime = 0;
    lastTeleportTime = 0;
}

void DashAura::onDisable() {
    targetList.clear();
}

bool DashAura::canAttack() {
    if(minCPS == 0 && maxCPS == 0)
        return true;

    static std::default_random_engine rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(minCPS, maxCPS);
    int cps = std::max(1, dist(rng));
    int delayMs = 1000 / cps;

    auto now = std::chrono::steady_clock::now();
    if(std::chrono::duration_cast<std::chrono::milliseconds>(now - lastAttack).count() >= delayMs) {
        lastAttack = now;
        return true;
    }
    return false;
}

Actor* DashAura::getNextTarget() {
    if(targetList.empty())
        return nullptr;
    return targetList[0];
}

void DashAura::attack(Actor* target) {
    if(!target)
        return;
    LocalPlayer* player = GI::getLocalPlayer();
    if(!player || !player->getgamemode())
        return;

    player->getgamemode()->attack(target);
    player->swing();
}

void DashAura::onNormalTick(LocalPlayer* player) {
    if(!player || !player->level)
        return;

    Level* level = player->level;
    BlockSource* region = GI::getRegion();
    targetList.clear();

    for(auto& entity : level->getRuntimeActorList()) {
        if(!TargetUtil::isTargetValid(entity, mobs2))
            continue;

        float checkRange = range;

        if(region->getSeenPercent(player->getEyePos(), entity->getAABB(true)) == 0.f &&
           !throughWallz)
            continue;

        if(player->getPos().dist(entity->getPos()) <= checkRange)
            targetList.push_back(entity);
    }

    if(targetList.empty())
        return;

    Actor* mainTarget = getNextTarget();
    Vec3<float> originalPos = player->getPos();
    Vec3<float> targetPos = mainTarget->getPos();

    auto now = std::chrono::steady_clock::now();
    std::int64_t currentTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    if(currentTime - lastTeleportTime >= (delay * 50)) {
        Vec3<float> newPos = targetPos;

        newPos.x += (rand() % 500 - 250) / 1000.f;
        newPos.z += (rand() % 500 - 250) / 1000.f;
        newPos.y += tpYOffset;

        Vec3<float> currentPos = player->getPos();
        Vec3<float> delta = {newPos.x - currentPos.x, newPos.y - currentPos.y,
                             newPos.z - currentPos.z};

        float distToTarget = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
        if(distToTarget > 0.001f) {
            float moveDist = std::min(tpMoveSpeed, distToTarget);
            float scale = moveDist / distToTarget;
            delta.x *= scale;
            delta.y *= scale;
            delta.z *= scale;
        }

        player->lerpMotion(delta);
        lastTeleportTime = currentTime;
    }

    if(currentTime - lastAttackTime >= (attackDelay * 50) && canAttack()) {
        attack(mainTarget);
        player->lerpMotion(originalPos - player->getPos());
        lastAttackTime = currentTime;
    }
}
