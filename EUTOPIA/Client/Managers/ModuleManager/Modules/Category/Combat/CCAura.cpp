#include <Minecraft/InvUtil.h>
#include <Minecraft/TargetUtil.h>
#include <Minecraft/WorldUtil.h>

#include "CCAura.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>
bool datimer;
int datimerspeed = 0;
#include "../../../ModuleManager.h"
#include "..\Utils\Minecraft\ActorUtils.h"

#define NOW                                                  \
    std::chrono::duration_cast<std::chrono::milliseconds>(   \
        std::chrono::system_clock::now().time_since_epoch()) \
        .count()

CCAura::CCAura() : Module("CCAura", "Aura for cubecraft", Category::COMBAT) {
    registerSetting(new BoolSetting("Timer", "Use timer for attacking", &datimer, false));
    registerSetting(new SliderSetting<int>("Speed", "Timer speed", &datimerspeed, 80, 10, 80));
    registerSetting(new SliderSetting<int>("Interval", "Attack delay (Ticks)", &interval, 1, 0, 20));
    registerSetting(new BoolSetting("Mobs", "Attack Mobs", &includeMobs, false));
}

void ccresettimertodefault() {

    if(auto ci = GI::getClientInstance()) {
        if(auto mc = ci->minecraftSim) {
            mc->setSimTimer(20.f);
            mc->setSimSpeed(20.f / 20.f);
        }
    }
}

void ccsettimertofast() {

    if(auto ci = GI::getClientInstance()) {
        if(auto mc = ci->minecraftSim) {
            mc->setSimTimer(datimerspeed);
            mc->setSimSpeed(datimerspeed / 20.f);
        }
    }

}

void CCAura::onEnable() {
    GI::DisplayClientMessage("[CCAura] Lets destroy cubecrap!");
    ccresettimertodefault();
    targetList.clear();
    shouldRotate = false;
}
void CCAura::onDisable() {
    ccresettimertodefault();
    targetList.clear();
    oTick = INT_MAX;
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(localPlayer)
        localPlayer->setSprinting(false);
}
bool CCAura::sortByDist(Actor* a1, Actor* a2) {
    Vec3<float> lpPos = GI::getLocalPlayer()->getPos();
    return a1->getPos().dist(lpPos) < a2->getPos().dist(lpPos);
}
int CCAura::getBestWeaponSlot(Actor* target) {
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
void CCAura::Attack(Actor* target) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer || !localPlayer->gamemode)
        return;
    localPlayer->gamemode->attack(target);
    localPlayer->swing();
}
bool CCAura::Counter(double a1) {
    if((GetTickCount64() - start) >= (a1 / timer)) {
        start = GetTickCount64();
        return true;
    }
    return false;
}
void CCAura::onNormalTick(LocalPlayer* player) {
    if(!player)
        return;
    Level* level = player->level;
    BlockSource* region = GI::getRegion();
    if(!region)
        return;

    if(datimer) {
        ccsettimertofast();
    } else {
        ccresettimertodefault();
    }
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
void CCAura::onUpdateRotation(LocalPlayer* player) {
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
        float distanceToTarget = playerPos.dist(targetPos);
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
        ActorRotationComponent* rotation = player->getActorRotationComponent();
        ActorHeadRotationComponent* headRot = player->getActorHeadRotationComponent();
        MobBodyRotationComponent* bodyRot = player->getMobBodyRotationComponent();
        rotation->mYaw = angle.y;
        rotation->mPitch = angle.x;
        rotation->mOldYaw = angle.y;
        rotation->mOldPitch = angle.x;
        headRot->mHeadRot = angle.y;
        bodyRot->yBodyRot = angle.y;
        player->setSprinting(true);
    }
    if(rotMode == 5 && !targetList.empty()) {
        Actor* target = targetList[0];
        Vec2<float> angle = player->getPos().CalcAngle(target->getPos()).normAngles();
        auto* rot = player->getActorRotationComponent();
        auto* head = player->getActorHeadRotationComponent();
        auto* body = player->getMobBodyRotationComponent();
        if(rot) {
            rot->mYaw = angle.y;
            rot->mPitch = angle.x;
            rot->mOldYaw = angle.y;
            rot->mOldPitch = angle.x;
        }
        if(head)
            head->mHeadRot = angle.y;
        if(body)
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
        auto* rot = player->getActorRotationComponent();
        auto* head = player->getActorHeadRotationComponent();
        auto* body = player->getMobBodyRotationComponent();
        if(rot) {
            rot->mYaw = angle.y;
            rot->mPitch = angle.x;
            rot->mOldYaw = angle.y;
            rot->mOldPitch = angle.x;
        }
        if(head)
            head->mHeadRot = angle.y;
        if(body)
            body->yBodyRot = angle.y;
        player->setSprinting(true);
    }
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
void CCAura::onLevelRender() {}