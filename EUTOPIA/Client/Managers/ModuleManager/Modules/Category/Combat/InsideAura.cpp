#include "InsideAura.h"

#include <Minecraft/TargetUtil.h>
#include <Minecraft/WorldUtil.h>

#include <algorithm>
#include <cmath>
#include <random>
// yes
bool throughWallz = false;
bool canSwing = true;
bool CrouchSneak = false;
bool sneakSpam = false;
bool stateSneak = false;
#include "../../../ModuleManager.h"

InsideAura::InsideAura()
    : Module("InsideAura", "Teleports inside entities and attacks them", Category::COMBAT) {
    registerSetting(new SliderSetting<float>("Reach", "Attack reach", &range, 5.f, 1.f, 16.f));
    registerSetting(new BoolSetting("Mobs", "Attack mobs", &includeMobs, false));
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
    registerSetting(new BoolSetting("UpAndDown", "Oscillate vertically", &upAndDown, false));
    registerSetting(
        new SliderSetting<float>("UpDown Speed", "Up/down speed", &upDownSpeed, 0.1f, 0.01f, 1.f));
    registerSetting(
        new SliderSetting<float>("UpDown Range", "Up/down range", &upDownRange, 1.f, 0.1f, 3.f));
    registerSetting(new BoolSetting("Through Walls", "Attack through walls", &throughWallz, false));
    registerSetting(new BoolSetting("Swing", "Play swing animation", &canSwing, true));
    registerSetting(new BoolSetting("Crouch", "Sneak while attacking", &CrouchSneak, false));
    registerSetting(new BoolSetting("Sneak spam", "Sneak spam while attacking", &sneakSpam, false));
}

void InsideAura::onEnable() {
    GI::DisplayClientMessage("[InsideAura] Enabled");
    targetList.clear();
    lastAttackTime = 0;
    lastTeleportTime = 0;
    upDownOffset = 0.f;
    upDownGoingUp = true;
    switchIndex = 0;
}

void InsideAura::onDisable() {
    GI::DisplayClientMessage("[InsideAura] Disabled");
    if(CrouchSneak) {
        GI::getLocalPlayer()->getMoveInputComponent()->setSneaking(false);
    }
    targetList.clear();
}

bool InsideAura::canAttack() {
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

Actor* InsideAura::getNextTarget() {
    if(targetList.empty())
        return nullptr;
    if(mode == 0)
        return targetList[0];
    if(mode == 2) {
        if(switchIndex >= targetList.size())
            switchIndex = 0;
        return targetList[switchIndex++];
    }
    return nullptr;
}

void InsideAura::attack(Actor* target) {
    if(!target)
        return;
    LocalPlayer* player = GI::getLocalPlayer();
    if(!player || !player->getgamemode())
        return;

    player->getgamemode()->attack(target);
    if(canSwing) {
        player->swing();
    }
}

void InsideAura::onNormalTick(LocalPlayer* player) {
    if(!player || !player->level)
        return;

 
    Level* level = player->level;
    BlockSource* region = GI::getRegion();
    targetList.clear();

    std::vector<Actor*> actors = ActorUtils::getActorList(false);
    for(auto* entity : actors) {
        if(!TargetUtil::isTargetValid(entity, includeMobs))
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

    std::sort(targetList.begin(), targetList.end(), [&](Actor* a, Actor* b) {
        return a->getPos().dist(player->getPos()) < b->getPos().dist(player->getPos());
    });

    Actor* mainTarget = targetList[0];
    Vec3<float> targetPos = mainTarget->getPos();

    if(CrouchSneak) {
        Game.getLocalPlayer()->getMoveInputComponent()->setSneaking(true);
    }

    if(sneakSpam) {
        stateSneak = !stateSneak;
        GI::getLocalPlayer()->getMoveInputComponent()->setSneaking(stateSneak);
    }

    

    auto now = std::chrono::steady_clock::now();
    std::int64_t currentTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    if(currentTime - lastTeleportTime >= (delay * 50)) {
        Vec3<float> newPos = targetPos;

        newPos.x += (rand() % 500 - 250) / 1000.f;
        newPos.z += (rand() % 500 - 250) / 1000.f;
        newPos.y += tpYOffset;

        if(upAndDown) {
            if(upDownGoingUp) {
                upDownOffset += upDownSpeed;
                if(upDownOffset >= upDownRange)
                    upDownGoingUp = false;
            } else {
                upDownOffset -= upDownSpeed;
                if(upDownOffset <= -upDownRange)
                    upDownGoingUp = true;
            }
            newPos.y += upDownOffset;
        }

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
        if(mode == 1) {
            for(auto& target : targetList)
                attack(target);
        } else {
            attack(getNextTarget());
        }
        lastAttackTime = currentTime;
    }
}
