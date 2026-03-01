#include <Minecraft/InvUtil.h>
#include <Minecraft/TargetUtil.h>
#include <Minecraft/WorldUtil.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>

#include "../../../../../../SDK/NetWork/Packets/MobEquipmentPacket.h"
#include "../../Client/Client.h"
#include "KillAura.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


#include "../../../../../../SDK/Render/MeshHelpers.h"
#include <DrawUtil.h>

KillAuraH::KillAuraH() : Module("KillAura", "Attacks entities around you", Category::COMBAT) {
    registerSetting(new SliderSetting<float>("Range", "Range in which targets will be hit", &range,
                                             5.f, 3.f, 150.f));
    registerSetting(
        new SliderSetting<float>("WallRange", "Range through walls", &wallRange, 0.f, 0.f, 150.f));
    registerSetting(
        new SliderSetting<float>("Predict", "Prediction distance", &predictvalue, 0.f, 0.f, 15.f));
    registerSetting(new SliderSetting<float>("Test", "Test value", &test, 0.f, 0.f, 15.f));
    registerSetting(new SliderSetting<float>("HeadSpeed", "Head rotation speed", &headspeed, 30.f,
                                             30.f, 360.f));
    registerSetting(new SliderSetting<int>("Interval", "Attack delay (Tick)", &interval, 1, 0, 20));
    registerSetting(
        new SliderSetting<int>("Multiplier", "Number of attacks per tick", &multiplier, 1, 1, 5));
    registerSetting(new EnumSetting("Rotations", "Rotates to targets",
                                    {"None", "Normal", "Strafe", "Predict"}, &rotMode, 1));
    registerSetting(new BoolSetting("Randomize", "Randomize attacks", &randomize, false));
    registerSetting(new EnumSetting("HitType", "Hit type", {"Single", "Multi"}, &hitType, 0));
    registerSetting(new SliderSetting<int>("HitChance", "Hit chance %", &hitChance, 100, 0, 100));
    registerSetting(new EnumSetting("Weapon", "Auto weapon switch", {"None", "Switch", "Spoof"},
                                    &autoWeaponMode, 0));
    registerSetting(
        new BoolSetting("Target Visualize", "Visualize targets", &targetVisualize, false));
    registerSetting(new ColorSetting("TV Color", "Visualization color", &visualizeColor,
                                     UIColor(255, 0, 0, 255)));
    registerSetting(new BoolSetting("Mobs", "Attack Mobs", &includeMobs, false));

    lastClearTime = std::chrono::steady_clock::now();
    attackCounter = 0;
    z = 0;
    Odelay = 0;
    oTick = 0;
}

std::string KillAuraH::getModeText() {
    switch(hitType) {
        case 0:
            return "Single";
        case 1:
            return "Multi";
        default:
            return "";
    }
}

void KillAuraH::onEnable() {
    targetList.clear();
    z = 0;
    Odelay = 0;
    oTick = 0;
    lastClearTime = std::chrono::steady_clock::now();
    attackCounter = 0;
    shouldRotate = false;
}

void KillAuraH::onDisable() {
    targetList.clear();
    attackCounter = 0;
    z = 0;
    Odelay = 0;
    oTick = 0;
    shouldRotate = false;
}

bool KillAuraH::sortByDist(Actor* a1, Actor* a2) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return false;

    Vec3<float> lpPos = localPlayer->getPos();
    return a1->getPos().dist(lpPos) < a2->getPos().dist(lpPos);
}

int KillAuraH::getBestWeaponSlot(Actor* target) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return localPlayer->supplies->mSelectedSlot;

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

bool KillAuraH::SpoofSwitch(int slot) {
    auto player = GI::getLocalPlayer();
    if(!player)
        return false;

    auto itemStack = player->supplies->container->getItem(slot);
    if(!itemStack)
        return false;

    auto mep = MinecraftPacket::createPacket(PacketID::MobEquipment);
    auto* pkt = reinterpret_cast<MobEquipmentPacket*>(mep.get());

    pkt->mSlot = slot;
    pkt->mSelectedSlot = slot;
    pkt->mContainerId = 0;
    pkt->mSlotByte = slot;
    pkt->mSelectedSlotByte = slot;
    pkt->mContainerIdByte = 0;

    auto packetSender = GI::getPacketSender();
    if(!packetSender)
        return false;

    packetSender->sendToServer(mep.get());
    return true;
}

void KillAuraH::Attack(Actor* target) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer || !localPlayer->gamemode)
        return;

    int randomNumber = rand() % 100;
    if(randomNumber < hitChance) {
        localPlayer->gamemode->attack(target);
    }
    localPlayer->swing();
}

void KillAuraH::clearTargetListIfNeeded() {
    auto now = std::chrono::steady_clock::now();
    auto timeSinceClear =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - lastClearTime).count();

    if(timeSinceClear > 1000 || attackCounter > 50) {
        targetList.clear();
        lastClearTime = now;
        attackCounter = 0;
    }
}

void KillAuraH::onNormalTick(LocalPlayer* localPlayer) {
    if(!localPlayer)
        return;

    Level* level = localPlayer->level;
    if(!level)
        return;

    clearTargetListIfNeeded();
    targetList.clear();

    std::vector<Actor*> allActors = localPlayer->level->getRuntimeActorList();
    for(auto* entity : allActors) {

        if(!TargetUtil::isTargetValid(entity, includeMobs))
            continue;

        float dist = localPlayer->getPos().dist(entity->getPos());
        if(dist <= range) {
            targetList.push_back(entity);
        }
    }

    if(targetList.empty()) {
        shouldRotate = false;
        return;
    }

    std::sort(targetList.begin(), targetList.end(), [](Actor* a1, Actor* a2) {
        LocalPlayer* localPlayer = GI::getLocalPlayer();
        if(!localPlayer)
            return false;

        Vec3<float> lpPos = localPlayer->getPos();
        return a1->getPos().dist(lpPos) < a2->getPos().dist(lpPos);
    });

    int oldSlot = localPlayer->supplies->mSelectedSlot;
    if(!targetList.empty()) {
        if(autoWeaponMode == 2) {
            SpoofSwitch(getBestWeaponSlot(targetList[0]));
            localPlayer->supplies->mSelectedSlot = getBestWeaponSlot(targetList[0]);
        } else if(autoWeaponMode == 1) {
            localPlayer->supplies->mSelectedSlot = getBestWeaponSlot(targetList[0]);
        }

        Vec3<float> targetPos = targetList[0]->getEyePos();
        rotAngle = localPlayer->getEyePos().CalcAngle(targetPos);
        rotAngle5 = localPlayer->getEyePos().CalcAngle(targetPos);

        shouldRotate = true;

        if(oTick >= interval) {
            if(hitType == 1) {
                for(auto& target : targetList) {
                    for(int j = 0; j < multiplier; ++j) {
                        Attack(target);
                    }
                }
            } else {
                for(int j = 0; j < multiplier; ++j) {
                    Attack(targetList[0]);
                }
            }
            oTick = 0;
            attackCounter++;
        } else {
            oTick++;
        }

        if(autoWeaponMode == 2) {
            localPlayer->supplies->mSelectedSlot = oldSlot;
        }
    }
}

void KillAuraH::onUpdateRotation(LocalPlayer* localPlayer) {
    if(!shouldRotate || !localPlayer || targetList.empty())
        return;

    ActorRotationComponent* rotation = localPlayer->getActorRotationComponent();
    ActorHeadRotationComponent* headRot = localPlayer->getActorHeadRotationComponent();
    MobBodyRotationComponent* bodyRot = localPlayer->getMobBodyRotationComponent();

    if(!rotation)
        return;

    switch(rotMode) {
        case 0:
            return;

        case 1: {
            if(rotation) {
                rotation->mPitch = rotAngle5.x;
                rotation->mYaw = rotAngle5.y;
            }
            if(headRot)
                headRot->mHeadRot = rotAngle5.y;
            if(bodyRot)
                bodyRot->yBodyRot = rotAngle5.y;
            break;
        }

        case 2: {
            if(rotation) {
                rotation->mPitch = rotAngle5.x;
                rotation->mYaw = rotAngle5.y;
            }
            break;
        }

        case 3: {
            float distanceXZ1 = localPlayer->getPos().dist(Vec3<float>(
                targetList[0]->getPos().x, localPlayer->getPos().y, targetList[0]->getPos().z));

            Actor* target = targetList[0];
            float targetYaw = target->getActorRotationComponent()->mYaw;
            float predictYaw = targetYaw + (headspeed - 90.0f);

            if(distanceXZ1 < predictvalue) {
                float predictRad = predictYaw * (M_PI / 180.0f);
                Vec3<float> targetPos = target->getPos();
                Vec3<float> backwardOffset(-cos(predictRad) * test, 0, -sin(predictRad) * test);
                Vec3<float> backwardPos =
                    Vec3<float>(targetPos.x + backwardOffset.x, targetPos.y + backwardOffset.y,
                                targetPos.z + backwardOffset.z);

                rotAngle = localPlayer->getPos().CalcAngle(backwardPos).normAngles();

                if(rotation) {
                    rotation->mYaw = rotAngle.y;
                    rotation->mPitch = rotAngle.x;
                }
            } else {
                Vec3<float> aimPos = targetList[0]->getEyePos();
                rotAngle = localPlayer->getPos().CalcAngle(aimPos).normAngles();

                if(rotation) {
                    rotation->mYaw = rotAngle.y;
                    rotation->mPitch = rotAngle.x;
                }
            }
            break;
        }
    }
}

void KillAuraH::onSendPacket(Packet* packet) {
    if(rotMode == 0 || !shouldRotate)
        return;

    if(packet->getId() == PacketID::PlayerAuthInput) {
        PlayerAuthInputPacket* paip = static_cast<PlayerAuthInputPacket*>(packet);
        paip->mRot = rotAngle;
        paip->mYHeadRot = rotAngle.y;
    }
}

void KillAuraH::onLevelRender() {
    LocalPlayer* player = GI::getLocalPlayer();
    if(player == nullptr || !targetVisualize || targetList.empty())
        return;

    auto* levelRenderer = GI::getClientInstance()->getLevelRenderer();
    if(!levelRenderer || !DrawUtil::tessellator || !DrawUtil::screenCtx || !DrawUtil::blendMaterial)
        return;

    Vec3<float> origin = levelRenderer->getrenderplayer()->origin;


    auto now = std::chrono::steady_clock::now();
    auto timeMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    float animation = sin(timeMs * 0.008f) * 0.2f;  

    for(Actor* target : targetList) {
        if(!target)
            continue;

        Vec3<float> targetPos = target->getPos();

        Vec3<float> circleCenter = {
            targetPos.x - origin.x,
            (targetPos.y - origin.y) - 0.8f + animation,  
            targetPos.z - origin.z};

        float radius = 0.7f;

        
        const int segments = 36;
        std::vector<Vec3<float>> circlePoints;
        circlePoints.reserve(segments + 1);

        for(int i = 0; i <= segments; ++i) {
            float angle = (i * 2.0f * M_PI) / segments;
            float x = circleCenter.x + radius * cosf(angle);
            float z = circleCenter.z + radius * sinf(angle);
            circlePoints.emplace_back(x, circleCenter.y, z);
        }

        bool isCurrentTarget = (target == targetList[0]);
        UIColor circleColor = visualizeColor;

       if(isCurrentTarget) {
            circleColor = UIColor(255, 255, 0, 255);  // Yellow for current target
        }

        UIColor lineColor = UIColor(circleColor.r, circleColor.g, circleColor.b, 255);

        
        DrawUtil::setColor(lineColor);
        DrawUtil::tessellator->begin(VertextFormat::LINE_STRIP, 2);

        for(const auto& point : circlePoints) {
            DrawUtil::tessellator->vertex(point.x, point.y, point.z);
        }

        MeshHelpers::renderMeshImmediately(DrawUtil::screenCtx, DrawUtil::tessellator,
                                           DrawUtil::blendMaterial);
    }
}