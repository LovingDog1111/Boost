#include "InfinityAura.h"

#include <FriendUtil.h>
#include <Minecraft/TargetUtil.h>
#include <Minecraft/WorldUtil.h>

#include "../../../ModuleManager.h"

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
int mode2882 = 0;


InfinityAura::InfinityAura()
    : Module("InfinityAura", "Attacks entities around you via teleport", Category::COMBAT) {
    registerSetting(new SliderSetting<float>("Range", "Attack range", &range, 5.f, 1.f, 100.f));
    registerSetting(new EnumSetting("Mode", "change mode", {"Lifeboat", "CubeCraft", "Normal"},
                                    &mode2882, 0));
    registerSetting(
        new SliderSetting<float>("Wall Range", "Through-wall range", &wallRange, 0.f, 0.f, 90.f));
    registerSetting(new BoolSetting("Mobs", "Include mobs", &includeMobs, false));
    registerSetting(new BoolSetting("Return After", "Return after attack", &shallReturn, false));
    registerSetting(new SliderSetting<float>("Attack Speed", "Attacks per second", &attackSpeed,
                                             5.f, 0.1f, 20.f));
    registerSetting(new SliderSetting<float>("Swing Speed", "Swing animation speed", &swingSpeed,
                                             5.f, 0.1f, 20.f));
    registerSetting(
        new SliderSetting<float>("TP Speed", "Teleport speed", &tpSpeed, 1.f, 0.1f, 5.f));
    registerSetting(new SliderSetting<int>("Multiplier", "Hits per tick", &multiplier, 1, 1, 5));
    registerSetting(new EnumSetting("Rotations", "Rotation mode",
                                    {"None", "Silent", "Strafe", "Short"}, &rotMode, 1));
    registerSetting(new BoolSetting("Body Yaw", "Use body yaw", &bodyYaw, false));
    registerSetting(new BoolSetting("Strafe", "Apply strafing rotation", &doStrafing, true));
    registerSetting(new BoolSetting("Raytrace Bypass", "Force ray hitbox", &rayTraceBypass, false));
    registerSetting(new BoolSetting("HvH Reset", "Reset on attack", &resetOnAttack, false));
    registerSetting(
        new SliderSetting<int>("Pitch Offset", "Pitch offset", &pitchOffset, 0, -180, 180));

}

void InfinityAura::onEnable() {
    targetList.clear();
    shouldRotate = false;
    moveState = ToTarget;
    step = 1;
    origin = Vec3<float>();
}

void InfinityAura::onDisable() {
    targetList.clear();
    shouldRotate = false;
    moveState = ToTarget;
    step = 1;
    origin = Vec3<float>();
}

void InfinityAura::onNormalTick(LocalPlayer* player) {
    if(!player || !player->level)
        return;

    BlockSource* region = GI::getRegion();
    targetList.clear();

    std::vector<Actor*> actors = player->level->getRuntimeActorList();
    for(auto* ent : actors) {
        if(!TargetUtil::isTargetValid(ent, includeMobs))
            continue;
        if(FriendManager::isFriend(ent->getNameTag()))
            continue;

        float checkRange = (region->getSeenPercent(player->getEyePos(), ent->getAABB(true)) == 0.f)
                               ? wallRange
                               : range;

        if(player->getPos().dist(ent->getPos()) <= checkRange)
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
    shouldRotate = rotMode != 0;

    auto now = std::chrono::steady_clock::now();
    float atkDelay = 1000.f / attackSpeed;
    float swingDelay = 1000.f / swingSpeed;

    if(std::chrono::duration_cast<std::chrono::milliseconds>(now - lastAttackTime).count() >=
       atkDelay) {
        for(int i = 0; i < multiplier; ++i)
            player->getgamemode()->attack(target);
        lastAttackTime = now;
    }

    if(std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSwingTime).count() >=
       swingDelay) {
        player->swing();
        lastSwingTime = now;
    }
}

void InfinityAura::onSendPacket(Packet* packet) {
    if(!packet || targetList.empty() || packet->getId() != PacketID::PlayerAuthInput)
        return;

    auto* paip = static_cast<PlayerAuthInputPacket*>(packet);
    if(!paip)
        return;

    LocalPlayer* player = GI::getLocalPlayer();
    if(!player)
        return;

    Actor* target = targetList[0];
    Vec3<float> currentPos = paip->mPos;

    Vec3<float> dir = currentPos.sub(target->getPos());
    dir = vec3Normalize(dir);
    if(mode2882 == 0) {
        Vec3<float> tpPos = target->getPos().add(dir * 7.f);
        paip->mPos = tpPos;
    }

     if(mode2882 == 1) {
        Vec3<float> tpPos = target->getPos().add(dir * 10.f);
        paip->mPos = tpPos;
    }

      if(mode2882 == 2) {
        Vec3<float> tpPos = target->getPos().add(dir * 7.f);
        paip->mPos = tpPos;
    }


    for(int i = 0; i < multiplier; ++i)
        player->getgamemode()->attack(target);

    player->swing();

    if(rotMode == 1 && shouldRotate) {
        paip->mRot.y = rotAngle.y;
        paip->mYHeadRot = rotAngle.x;
    }

    if(rayTraceBypass) {
        HitResult* hit = player->level->getHitResult();
        if(hit) {
            Vec3<float> eye = target->getEyePos();
            eye.y = target->getAABB(true).getCenter().y;
            hit->type = HitResultType::ENTITY;
            hit->startPos = eye;
            hit->endPos = eye;
            hit->rayDirection = eye;
            hit->blockPos = Vec3<int>(eye.toInt());
        }
    }
}

void InfinityAura::onUpdateRotation(LocalPlayer* player) {
    if(!shouldRotate || rotMode == 0)
        return;

    auto* rot = player->getActorRotationComponent();
    auto* body = player->getMobBodyRotationComponent();
    auto* head = player->getActorHeadRotationComponent();
    if(!rot || !body || !head)
        return;

    if(rotMode == 3 && !targetList.empty()) {
        Vec3<float> aim = targetList[0]->getEyePos();
        aim.y = targetList[0]->getAABB(true).getCenter().y;
        rotAngle = player->getEyePos().CalcAngle(aim);
    }

    if(rotMode == 2 || rotMode == 3) {
        rot->mYaw = rotAngle.y;
        rot->mPitch = rotAngle.x + pitchOffset;
        rot->mOldYaw = rot->mYaw;
        rot->mOldPitch = rot->mPitch;

        if(bodyYaw)
            body->yBodyRot = rotAngle.y;
        head->mHeadRot = rotAngle.y;
    }
}