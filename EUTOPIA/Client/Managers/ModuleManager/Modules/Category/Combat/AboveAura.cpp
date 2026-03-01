#include "AboveAura.h"

#include <FriendUtil.h>
#include <Minecraft/TargetUtil.h>
#include <Minecraft/WorldUtil.h>

#include "../../../ModuleManager.h"
bool swing = true;
float yOffset = 1.f;
bool sneak = false;
template <typename T>
float vec3Length(const Vec3<T>& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

template <typename T>
Vec3<T> vec3Normalize(const Vec3<T>& v) {
    float len = vec3Length(v);
    if(len == 0.0f)
        return Vec3<T>{0, 0, 0};
    return Vec3<T>{v.x / len, v.y / len, v.z / len};
}

AboveAura::AboveAura()
    : Module("AboveAura", "Teleport continuously above the target while attacking",
             Category::COMBAT) {
    registerSetting(new SliderSetting<float>("Range", "Attack range", &range, 5.f, 1.f, 15.f));
    registerSetting(new BoolSetting("Include Mobs", "Include mobs", &includeMobs, false));
    registerSetting(new BoolSetting("Return After", "Return after attack", &shallReturn, false));
    registerSetting(new SliderSetting<float>("Attack Speed", "Attacks per second", &attackSpeed,
                                             5.f, 0.1f, 25.f));
    registerSetting(
        new SliderSetting<float>("Teleport Speed", "Teleport speed", &tpSpeed, 1.f, 0.1f, 5.f));
    registerSetting(new SliderSetting<float>("Y Offset", "Vertical offset above target", &yOffset,
                                             0.5f, 0.0f, 5.0f));
    registerSetting(new BoolSetting("Reset On Attack", "Reset teleport steps on attack",
                                    &resetOnAttack, false));
    registerSetting(new BoolSetting("Swing", "Play swing animation",
                                    &swing, true));
    registerSetting(new BoolSetting("Crouch", "Sneak while attacking", &sneak, false));
    registerSetting(new BoolSetting("UpAndDown", "Oscillate vertically", &upAndDown, false));
    registerSetting(new SliderSetting<float>("UpDown Speed", "Vertical speed for oscillation",
                                             &upDownSpeed, 0.1f, 0.01f, 2.f));
    registerSetting(new SliderSetting<float>("UpDown Range", "Vertical range for oscillation",
                                             &upDownRange, 1.f, 0.1f, 5.f));
}

void AboveAura::onEnable() {
    GI::DisplayClientMessage("[AboveAura] Enabled");
    targetList.clear();
    shouldRotate = false;
    moveState = ToTarget;
    step = 1;
    origin = Vec3<float>();
    upDownOffset = 0.f;
    upDownGoingUp = true;
}

void AboveAura::onDisable() {
    GI::DisplayClientMessage("[AboveAura] Disabled");
    if(sneak) {
        GI::getLocalPlayer()->getMoveInputComponent()->setSneaking(false);
    }
    targetList.clear();
    shouldRotate = false;
    moveState = ToTarget;
    step = 1;
    origin = Vec3<float>();
}

void AboveAura::onNormalTick(LocalPlayer* player) {
    if(!player || !player->level)
        return;

    if(sneak) {
        GI::getLocalPlayer()->getMoveInputComponent()->setSneaking(true);
    }

    targetList.clear();
    std::vector<Actor*> actors = player->level->getRuntimeActorList();
    for(auto* ent : actors) {
        if(!TargetUtil::isTargetValid(ent, includeMobs))
            continue;
        if(FriendManager::isFriend(ent->getNameTag()))
            continue;
        if(player->getPos().dist(ent->getPos()) <= range)
            targetList.push_back(ent);
    }

    if(targetList.empty()) {
        shouldRotate = false;
        return;
    }

    std::sort(targetList.begin(), targetList.end(), [&](Actor* a, Actor* b) {
        return a->getPos().dist(player->getPos()) < b->getPos().dist(player->getPos());
    });

    Actor* target = targetList[0];
    Vec3<float> aim = target->getEyePos();
    aim.y = target->getAABB(true).getCenter().y;

    rotAngle = player->getEyePos().CalcAngle(aim);
    shouldRotate = true;

    auto now = std::chrono::steady_clock::now();
    float atkDelay = 1000.f / attackSpeed;

    if(std::chrono::duration_cast<std::chrono::milliseconds>(now - lastAttackTime).count() >=
       atkDelay) {
        player->getgamemode()->attack(target);
        if(swing) {
            player->swing();
        }
        lastAttackTime = now;
    }
}

void AboveAura::onSendPacket(Packet* packet) {
    if(!packet || targetList.empty() || packet->getId() != PacketID::PlayerAuthInput)
        return;

    auto* paip = static_cast<PlayerAuthInputPacket*>(packet);
    if(!paip)
        return;

    LocalPlayer* player = GI::getLocalPlayer();
    if(!player)
        return;

    Actor* target = targetList[0];
    Vec3<float> currentPos = player->getPos();
    Vec3<float> targetPos = target->getAABB(true).getCenter();
    targetPos.y += yOffset;

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
        targetPos.y += upDownOffset;
    }

    Vec3<float> dest = (moveState == ToTarget) ? targetPos : origin;

    float dist = currentPos.dist(dest);

    if(dist <= 0.0f) {
        step = 1;
        if(shallReturn) {
            moveState = (moveState == ToTarget) ? Returning : ToTarget;
            dest = (moveState == ToTarget) ? targetPos : origin;
            dist = currentPos.dist(dest);
        } else {
            return;
        }
    }

    if(dist <= 0.0f)
        return;

    float stepSize = tpSpeed * step;
    float t = std::min(stepSize / dist, 1.0f);

    Vec3<float> newPos = currentPos.lerpTo(dest, t);

    player->teleportTo(newPos, false, 1, 1, false);

    step++;
    if(step > static_cast<int>(12 / tpSpeed) && resetOnAttack)
        step = 1;
}

void AboveAura::onUpdateRotation(LocalPlayer* player) {
    if(!shouldRotate)
        return;

}

