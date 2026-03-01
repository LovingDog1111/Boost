#include <Minecraft/InvUtil.h>
#include <Minecraft/TargetUtil.h>
#include <Minecraft/WorldUtil.h>

#include "KillAuraV2.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>
#include "..\Utils\Minecraft\ActorUtils.h"
#include "../../../ModuleManager.h"

#define NOW                                                  \
    std::chrono::duration_cast<std::chrono::milliseconds>(   \
        std::chrono::system_clock::now().time_since_epoch()) \
        .count()

KillAura2::KillAura2() : Module("KillAura-v2", "Attacks entities around you", Category::COMBAT) {
    registerSetting(new SliderSetting<float>("Range", "Range in which targets will be hit", &range,
                                             5.f, 3.f, 150.f));
    registerSetting(new SliderSetting<int>("Interval", "Attack delay (Tick)", &interval, 1, 0, 20));
    registerSetting(
        new SliderSetting<int>("Multiplier", "Number of attacks per tick", &multiplier, 1, 1, 5));
    registerSetting(new SliderSetting<float>("APS", "Attacks per second", &aps, 1.f, 1.f, 5000.f));
    registerSetting(new EnumSetting(
        "Rotation", "",
        {"None", "Strafe", "Randomize", "Star", "Circle", "StrafeV2", "StrafeV3", "Simple"},
        &rotMode, 0));
    registerSetting(new BoolSetting("Mobs", "Attack Mobs", &includeMobs, false));
}
void KillAura2::onEnable() {
    targetList.clear();
    shouldRotate = false;
}
void KillAura2::onDisable() {
    targetList.clear();
    oTick = INT_MAX;
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(localPlayer)
        localPlayer->setSprinting(false);
}
bool KillAura2::sortByDist(Actor* a1, Actor* a2) {
    Vec3<float> lpPos = GI::getLocalPlayer()->getPos();
    return a1->getPos().dist(lpPos) < a2->getPos().dist(lpPos);
}
int KillAura2::getBestWeaponSlot(Actor* target) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return 0;
    int bestSlot = InvUtil::getSelectedSlot();
    float highestDamage = 0.f;
    for(int i = 0; i < 9; i++) {
        ItemStack* item = InvUtil::getItem(i);
        if(!InvUtil::isVaildItem(item))
            continue;
        float itemDamage = 1.0f;
        std::string itemName = InvUtil::getItemName(item);
        if(itemName.find("Sword") != std::string::npos)
            itemDamage = 6.0f;
        else if(itemName.find("Axe") != std::string::npos)
            itemDamage = 7.0f;
        else if(itemName.find("Pickaxe") != std::string::npos)
            itemDamage = 5.0f;
        else if(itemName.find("Shovel") != std::string::npos)
            itemDamage = 4.0f;
        itemDamage *= (1.0f + (1.0f - InvUtil::caculateItemDurability(item)));
        if(itemDamage > highestDamage) {
            highestDamage = itemDamage;
            bestSlot = i;
        }
    }
    return bestSlot;
}
void KillAura2::Attack(Actor* target) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer || !localPlayer->gamemode)
        return;
    localPlayer->gamemode->attack(target);
    localPlayer->swing();
}
bool KillAura2::Counter(double a1) {
    if((GetTickCount64() - start) >= (a1 / timer)) {
        start = GetTickCount64();
        return true;
    }
    return false;
}
void KillAura2::onNormalTick(LocalPlayer* player) {
    if(!player)
        return;
    Level* level = player->level;
    BlockSource* region = GI::getRegion();
    if(!region)
        return;
    targetList.clear();
    std::vector<Actor*> actors = player->level->getRuntimeActorList();
    for(auto* entity : actors) {
        if(!TargetUtil::isTargetValid(entity, includeMobs))
            continue;
        if(player->getPos().dist(entity->getPos()) <= range)
            targetList.push_back(entity);
    }
    if(targetList.empty()) {
        shouldRotate = false;
        return;
    }
    std::sort(targetList.begin(), targetList.end(), sortByDist);
    if(rotMode != 0) {
        Vec3<float> targetPos = targetList[0]->getEyePos();
        rotAngle = player->getEyePos().CalcAngle(targetPos);
        shouldRotate = true;
    } else {
        shouldRotate = false;
    }
    static int64_t lastAttack = 0;
    if(Counter(1000.0 / aps)) {
        for(int j = 0; j < multiplier; ++j) {
            Attack(targetList[0]);
        }
    }
}
void KillAura2::onUpdateRotation(LocalPlayer* player) {
    if(!shouldRotate || rotMode == 0)
        return;
    auto* rot = player->getActorRotationComponent();
    auto* head = player->getActorHeadRotationComponent();
    auto* body = player->getMobBodyRotationComponent();
    Vec2<float> finalAngle = rotAngle;
    if(rotMode == 2) {
        static std::default_random_engine rng(std::random_device{}());
        std::uniform_real_distribution<float> yawJitter(-3.f, 3.f);
        std::uniform_real_distribution<float> pitchJitter(-2.f, 2.f);
        finalAngle.y += yawJitter(rng);
        finalAngle.x += pitchJitter(rng);
        finalAngle.x = std::clamp(finalAngle.x, -89.f, 89.f);
    }
    if(rotMode == 3 && !targetList.empty()) {
        static float t = 0.f;
        t += 0.15f;
        float angle = t * 2.f * M_PI / 5.f;
        float r = fmodf(t * 2.f, 2.f) < 1.f ? 8.f : 5.f;
        float offsetX = cos(angle) * r;
        float offsetZ = sin(angle) * r;
        Vec3<float> targetPos = targetList[0]->getPos();
        Vec3<float> newPos = targetPos + Vec3<float>(offsetX, 0, offsetZ);
        player->SetPosition(Game.getLocalPlayer()->getStateVectorComponent(), newPos);
    }
    if(rotMode == 4 && !targetList.empty()) {
        Actor* target = targetList[0];
        Vec3<float> targetPos = target->getPos();
        Vec3<float> playerPos = player->getPos();
        static float circleAngle = 0.f;
        circleAngle += 0.05f;
        if(circleAngle > 2 * M_PI)
            circleAngle -= 2 * M_PI;
        float radius = 2.5f;
        float offsetX = cos(circleAngle) * radius;
        float offsetZ = sin(circleAngle) * radius;
        Vec3<float> moveDir = (targetPos + Vec3<float>(offsetX, 0, offsetZ)) - playerPos;
        moveDir.y = 0;
        float speed = 0.3f;
        float len = sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
        if(len > 0.001f)
            moveDir =
                Vec3<float>(moveDir.x / len * speed, player->getStateVectorComponent()->mVelocity.y,
                            moveDir.z / len * speed);
        player->getStateVectorComponent()->mVelocity = moveDir;
        Vec2<float> angle = playerPos.CalcAngle(targetPos).normAngles();
        rot->mYaw = angle.y;
        rot->mPitch = angle.x;
        rot->mOldYaw = angle.y;
        rot->mOldPitch = angle.x;
        head->mHeadRot = angle.y;
        body->yBodyRot = angle.y;
        player->setSprinting(true);
    }
    if(rotMode == 5 && !targetList.empty()) {
        Actor* target = targetList[0];
        Vec2<float> angle = player->getPos().CalcAngle(target->getPos()).normAngles();
        rot->mYaw = angle.y;
        rot->mPitch = angle.x;
        rot->mOldYaw = angle.y;
        rot->mOldPitch = angle.x;
        head->mHeadRot = angle.y;
        body->yBodyRot = angle.y;
    }
    if(rotMode == 6 && !targetList.empty()) {
        Actor* target = targetList[0];
        Vec3<float> targetPos = target->getPos();
        Vec3<float> playerPos = player->getPos();
        static float circleAngle = 0.f;
        circleAngle += 0.05f;
        if(circleAngle > 2 * M_PI)
            circleAngle -= 2 * M_PI;
        float radius = 2.5f;
        float offsetX = cos(circleAngle) * radius;
        float offsetZ = sin(circleAngle) * radius;
        Vec3<float> moveDir = (targetPos + Vec3<float>(offsetX, 0, offsetZ)) - playerPos;
        moveDir.y = 0;
        float speed = 0.3f;
        float len = sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
        if(len > 0.001f)
            moveDir =
                Vec3<float>(moveDir.x / len * speed, player->getStateVectorComponent()->mVelocity.y,
                            moveDir.z / len * speed);
        player->getStateVectorComponent()->mVelocity = moveDir;
        Vec2<float> angle = playerPos.CalcAngle(targetPos).normAngles();
        rot->mYaw = angle.y;
        rot->mPitch = angle.x;
        rot->mOldYaw = angle.y;
        rot->mOldPitch = angle.x;
        head->mHeadRot = angle.y;
        body->yBodyRot = angle.y;
        player->setSprinting(true);
    }
    if(rotMode == 7 && !targetList.empty()) {
        Actor* target = targetList[0];
        Vec3<float> targetPos = target->getEyePos();
        Vec3<float> playerPos = player->getEyePos();
        Vec2<float> desiredAngle = playerPos.CalcAngle(targetPos).normAngles();

        float headSmooth = 0.15f;  
        float bodySmooth = 0.08f;

       
        float yawDiff = desiredAngle.y - rot->mYaw;
        while(yawDiff > 180.f)
            yawDiff -= 360.f;
        while(yawDiff < -180.f)
            yawDiff += 360.f;
        rot->mYaw += yawDiff * headSmooth;

       
        float pitchDiff = desiredAngle.x - rot->mPitch;
        rot->mPitch += pitchDiff * headSmooth;

       
        rot->mOldYaw = rot->mYaw;
        rot->mOldPitch = rot->mPitch;

        if(head)
            head->mHeadRot = rot->mYaw;

        
        if(body) {
            float bodyYawDiff = rot->mYaw - body->yBodyRot;
            while(bodyYawDiff > 180.f)
                bodyYawDiff -= 360.f;
            while(bodyYawDiff < -180.f)
                bodyYawDiff += 360.f;
            body->yBodyRot += bodyYawDiff * bodySmooth;
        }
    } else {
      
        if(rot) {
            rot->mYaw = finalAngle.y;
            rot->mPitch = finalAngle.x;
            rot->mOldYaw = finalAngle.y;
            rot->mOldPitch = finalAngle.x;
        }
        if(head)
            head->mHeadRot = finalAngle.y;
        if(body)
            body->yBodyRot = finalAngle.y;
    }
}

void KillAura2::onLevelRender() {}