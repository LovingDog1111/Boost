#include <Minecraft/InvUtil.h>
#include <Minecraft/TargetUtil.h>
#include <Minecraft/WorldUtil.h>

#include "Aura.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>

#include "../../../ModuleManager.h"
#include "../../../../../../SDK/NetWork/Packets/InteractPacket.h"
#include <DirectXMath.h>
int hitType;
int rotationAngle = 180;
bool osurots;
bool flickrots;
float targetYaw = 0.f;
float currentYaw = 0.f;
float ysex = 0.f;
int packets;
int hitChance;
int autoWeaponMode;
#define NOW                                                  \
    std::chrono::duration_cast<std::chrono::milliseconds>(   \
        std::chrono::system_clock::now().time_since_epoch()) \
        .count()

KillAura3::KillAura3() : Module("KillAura-v3", "Aura for 2b2tpe", Category::COMBAT) {
    registerSetting(new SliderSetting<float>("Range", "Range in which targets will be hit", &range,
                                             5.f, 3.f, 150.f));
    registerSetting(new EnumSetting("HitType", "NULL", {"Single", "Multi"}, &hitType, 0));
    registerSetting(
        new EnumSetting("delay method", "method of delay", {"delay", "cps"}, &methodFE, 0));
    registerSetting(new SliderSetting<int>("HitChance", "NULL", &hitChance, 100, 0, 100));
    registerSetting(new SliderSetting<int>("Interval", "Attack delay (Tick)", &interval, 1, 0, 20));
    registerSetting(new SliderSetting<int>("cps", "cps", &cps, 10, 1, 100));
    registerSetting(new SliderSetting<int>(
        "cps-divider", "best not to mess with. might bypass max per tick", &cpstest, 20, 0, 100));
    registerSetting(
        new SliderSetting<int>("Attemps", "Number of attacks per tick", &multiplier, 1, 1, 50));
    registerSetting(
        new SliderSetting<int>("Packets", "Amount of attack packets", &packets, 3, 0, 50));
    registerSetting(new BoolSetting("boost", "randomly attack really fast", &boost, false));
    registerSetting(new SliderSetting<float>("boost amount", "how much to boost attack speed",
                                             &boostAmount, 2.0f, 1.0f, 10.0f));
    registerSetting(new SliderSetting<float>("boost delay", "seconds between boosts", &boostDelay,
                                             5.0f, 0.1f, 60.0f));

    registerSetting(new EnumSetting("Rotation", "", {"None", "Rotation", "Strafev2" },
        &rotMode, 0));
    registerSetting(new SliderSetting<float>("Rotation Speed", "Yaw Rotation speed", &rotationSpeed,
                                             0.01f, 0.f, 13.0f));
    registerSetting(
        new SliderSetting<int>("Rotatin Angle", "Angle", &rotationAngle, 180, 0, 10000));
    registerSetting(
        new SliderSetting<float>("Rotation yOffset", "Change the y offset", &ysex,
                                             0.f, -5.f, 5.f));
    registerSetting(new EnumSetting("SwitchMode", "Auto switch to best weapon",
                                    {"None", "Switch", "Spoof"}, &autoWeaponMode, 0, 3));
    registerSetting(new BoolSetting("OsuRots", "diff rot", &osurots, false));
    registerSetting(new BoolSetting("FlickRots", "HeadRots", &flickrots, false));
    registerSetting(new BoolSetting("Mobs", "Attack Mobs", &includeMobs, false));
}
void KillAura3::onEnable() {
    targetList.clear();
    shouldRotate = false;
    targetYaw = 0.f;
    isBoosting = false;
    boostTimer = 0.f;
    lastBoostTime = 0.f;
}
void KillAura3::onDisable() {
    targetList.clear();
    oTick = INT_MAX;
    targetYaw = 0.f;
    isBoosting = false;
    boostTimer = 0.f;
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(localPlayer)
        localPlayer->setSprinting(false);
}
bool KillAura3::sortByDist(Actor* a1, Actor* a2) {
    Vec3<float> lpPos = GI::getLocalPlayer()->getPos();
    return a1->getPos().dist(lpPos) < a2->getPos().dist(lpPos);
}

int KillAura3::getBestWeaponSlot(Actor* target) {
    LocalPlayer* localPlayer = Game.getLocalPlayer();
    Container* inventory = localPlayer->supplies->container;
    float damage = 0.f;
    int slot = localPlayer->supplies->mSelectedSlot;
    for(int i = 0; i < 9; i++) {
        localPlayer->supplies->mSelectedSlot = i;
        float currentDamage = localPlayer->calculateDamage(target);
        if(currentDamage > damage) {
            damage = currentDamage;
            slot = i;
        }
    }
    return slot;
}


int KillAura3::bestslot(Actor* target) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer || !localPlayer->gamemode) {
        return InvUtil::getSelectedSlot();
    }

    int fallback = getBestWeaponSlot(target);

    if(localPlayer->IsinWaterOrRain()) {
        int tridentSlot = InvUtil::findItemSlotInHotbar(579);  
        if(tridentSlot != -1)                                 
            return tridentSlot;
    }

    return fallback;
}

void KillAura3::Attack(Actor* target) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer || !localPlayer->gamemode)
        return;
    int randomNumber = (int)(rand() % 100);
    if(randomNumber < hitChance) {
        localPlayer->gamemode->attack(target);
    }
    localPlayer->swing();
}
void KillAura3::onNormalTick(LocalPlayer* player) {
    if(player == nullptr)
        return;
    BlockSource* region = Game.getClientInstance()->getRegion();
    Level* level = player->level;

    float deltaTime = RenderUtil::deltaTime;
    if(boost) {
        if(!isBoosting) {
            lastBoostTime += deltaTime;
            if(lastBoostTime >= boostDelay) {
                isBoosting = true;
                boostTimer = 1.0f;
                lastBoostTime = 0.f;
            }
        } else {
            boostTimer -= deltaTime;
            if(boostTimer <= 0.f) {
                isBoosting = false;
            }
        }
    }
    if(methodFE == 1) {
        if(cps <= 0)
            return;
    }
    auto& inventory = *player->supplies;
    if(isBoosting) {
        interval =
            (methodFE == 0) ? (oTick / boostAmount) : (cpstest / (cps * boostAmount));
    } else {
        interval = (methodFE == 0) ? oTick : (cpstest / cps);
    }
    if(oTick++ >= interval) {
        oTick = 0;
        for(Actor* ent : player->getlevel()->getRuntimeActorList()) {
            if(!TargetUtil::isTargetValid(ent, includeMobs))
                continue;
            if(player->getPos().dist(ent->getPos()) > range)
                continue;
            const int oldSlot = inventory.mSelectedSlot;
            if(autoWeaponMode != 0) {
                inventory.mSelectedSlot = bestslot(ent);
            }
            int attacks = isBoosting ? packets : multiplier;
            for(int i = 0; i < attacks; i++) {
                Attack(ent);
                attacking12 = true; 
            }
            if(autoWeaponMode == 2) {
                inventory.mSelectedSlot = oldSlot;
            }
            if(hitType == 0)
                break;
        }
    } else {
        attacking12 = false;
    }
}

void KillAura3::onUpdateRotation(LocalPlayer* localPlayer) {
    if(rotMode != 1)
        return;
    if(flickrots && oTick < interval)
        return;
    Level* level = localPlayer->level;
    auto* headRot = localPlayer->getActorHeadRotationComponent();

    for(Actor* ent : localPlayer->getlevel()->getRuntimeActorList()) {
        if(!TargetUtil::isTargetValid(ent, includeMobs))
            continue;
        if(localPlayer->getPos().dist(ent->getPos()) > range)
            continue;
        Vec2<float> targetRot =
            localPlayer->getEyePos().CalcAngle(ent->getEyePos().add(Vec3<float>(0.f, ysex, 0.f)));
        if(rotMode == 1) {
            targetYaw = targetRot.y;
            float angleStep = 360.0f / rotationAngle;
            float snappedYaw = std::round(targetYaw / angleStep) * angleStep;
            float angleDiff = snappedYaw - currentYaw;
            while(angleDiff < -180.f)
                angleDiff += 360.f;
            while(angleDiff > 180.f)
                angleDiff -= 360.f;
            currentYaw += angleDiff * (0.1f * rotationSpeed);
            rotAngle.x = targetRot.x;
            rotAngle.y = currentYaw;
        } else {
                Actor* target = targetList[0];
                auto* rot = localPlayer->getActorRotationComponent();
            auto* head = localPlayer->getActorHeadRotationComponent();
                auto* body = localPlayer->getMobBodyRotationComponent();
                if(rot) {
                    rot->mYaw = targetRot.y;
                    rot->mPitch = targetRot.x;
                    rot->mOldYaw = targetRot.y;
                    rot->mOldPitch = targetRot.x;
                }
                if(head)
                    head->mHeadRot = targetRot.y;
                if(body)
                    body->yBodyRot = targetRot.y;
        }
        if(osurots) {
            constexpr float snapDivisor = 36.4f;
            rotAngle.x = std::round(rotAngle.x / snapDivisor) * snapDivisor;
            rotAngle.y = std::round(rotAngle.y / snapDivisor) * snapDivisor;
        }
        if(headRot) {
            localPlayer->getActorRotationComponent()->mPitch = rotAngle.x;
            localPlayer->getActorRotationComponent()->mYaw= rotAngle.y;
            headRot->mHeadRot = rotAngle.y;
        }
        if(auto* bodyRot = localPlayer->getMobBodyRotationComponent()) {
            bodyRot->yBodyRot = rotAngle.y;
        }
        break;
    }
}
void KillAura3::onSendPacket(Packet* packet) {
    auto localPlayer = Game.getLocalPlayer();
    if(localPlayer == nullptr)
        return;
    if(packet->getId() == PacketID::Interact) {
        for(int i = 0; i < packets; i++) {
            for(Actor* ent : Game.getLocalPlayer()->getlevel()->getRuntimeActorList()) {
                if(!TargetUtil::isTargetValid(ent, includeMobs))
                    continue;
                if(Game.getLocalPlayer()->getPos().dist(ent->getPos()) > range)
                    continue;
                InteractPacket* interact = static_cast<InteractPacket*>(packet);
                interact->mAction = InteractPacket::Action::LeftClick;
                interact->mTargetId = ent->getActorTypeComponent()->id;
                interact->mPos = ent->getPos();
                Game.getLocalPlayer()->swing();
                if(boost) {
                    Attack(ent);
                }
            }
        }
    }
}
void KillAura3::onLevelRender() {}

