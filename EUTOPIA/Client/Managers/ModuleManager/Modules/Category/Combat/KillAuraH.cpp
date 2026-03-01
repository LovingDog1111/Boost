#include "KillAuraH.h"

#include <DrawUtil.h>
#include <Minecraft/InvUtil.h>
#include <Minecraft/TargetUtil.h>
#include <Minecraft/WorldUtil.h>

#include "../../../../../../SDK/Render/MeshHelpers.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <Windows.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>
#include <string>
#include <vector>

#include "../../../ModuleManager.h"

KillAuraH2::KillAuraH2()
    : Module("KillAuraH", "Advanced KillAura with Hook Speed", Category::COMBAT) {
    registerSetting(new SliderSetting<float>("Range", "Attack range", &range, 5.f, 3.f, 150.f));
    registerSetting(new SliderSetting<float>("APS", "Attacks per second", &aps, 10.f, 1.f, 1000.f));
    registerSetting(
        new BoolSetting("EnableHookSpeed", "Enable hook speed system", &enableHookSpeed, true));
    registerSetting(
        new BoolSetting("AutoHookSpeed", "Auto hook speed patterns", &autoHookSpeed, false));
    registerSetting(
        new SliderSetting<int>("HookAmount", "Number of hook attacks", &hookAmount, 5, 1, 20));
    registerSetting(
        new SliderSetting<int>("Multiplier", "Extra attacks per hook", &multiplier, 1, 1, 10));
    registerSetting(new EnumSetting("SpeedType", "Hook speed pattern",
                                    {"Fast", "Slow", "Medium", "Random"}, &speedType, 0));
    registerSetting(new EnumSetting("Rotation", "Rotation mode",
                                    {"None", "Normal", "Strafe", "Predict"}, &rotMode, 1));
    registerSetting(new BoolSetting("Target Strafe", "Strafe around target", &strafe, false));
    registerSetting(
        new SliderSetting<float>("Test-Strafe", "Test-Strafe distance", &test, 0.f, 0.f, 15.f));
    registerSetting(new SliderSetting<float>("Test", "Test value", &headspeed, 30.f, 0.f, 360.f));
    registerSetting(new BoolSetting("Mobs", "Attack Mobs", &includeMobs, false));
    registerSetting(new BoolSetting("VisualTarget", "Show target highlight", &visualTarget, false));
    registerSetting(new BoolSetting("VisualRange", "Show attack range", &visualRange, false));
    registerSetting(
        new ColorSetting("TargetColor", "Target color", &targetColor, UIColor(255, 0, 0, 255)));
}

void KillAuraH2::onEnable() {
    targetList.clear();
    shouldRotate = false;
    attackCounter = 0;
    hookCounter = 0;
    autoSpeedCounter = 0;
    start = GetTickCount64();
    lastAttackTime = 0;
}

void KillAuraH2::onDisable() {
    targetList.clear();
    shouldRotate = false;
}

bool KillAuraH2::sortByDist(Actor* a1, Actor* a2) {
    if(!a1 || !a2 || !GI::getLocalPlayer())
        return false;
    Vec3<float> lpPos = GI::getLocalPlayer()->getPos();
    return a1->getPos().dist(lpPos) < a2->getPos().dist(lpPos);
}

void KillAuraH2::Attack(Actor* target) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer || !localPlayer->gamemode || !target)
        return;
    localPlayer->gamemode->attack(target);
    localPlayer->swing();
    attackCounter++;
}

bool KillAuraH2::Counter(double delayMs) {
    if(aps <= 0.0f)
        return false;
    if((GetTickCount64() - start) >= delayMs / aps) {
        start = GetTickCount64();
        return true;
    }
    return false;
}

void KillAuraH2::updateAutoHookSpeed() {
    if(!autoHookSpeed)
        return;

    autoSpeedCounter++;
    switch(speedType) {
        case 0:
            hookAmount = 6 + (autoSpeedCounter % 7);
            break;
        case 1:
            hookAmount = 2 + (autoSpeedCounter % 5);
            break;
        case 2:
            hookAmount = 5 + (autoSpeedCounter % 4);
            break;
        case 3:
            hookAmount = rand() % 15 + 1;
            break;
        default:
            break;
    }

    if(autoSpeedCounter > 1000)
        autoSpeedCounter = 0;
}

void KillAuraH2::onNormalTick(LocalPlayer* player) {
    if(!player || !player->level)
        return;

    targetList.clear();
    auto actors = player->level->getRuntimeActorList();

    for(auto* entity : actors) {
        if(!entity)
            continue;
        if(!TargetUtil::isTargetValid(entity, includeMobs))
            continue;
        if(!entity->isAlive())
            continue;
        if(player->getPos().dist(entity->getPos()) <= range)
            targetList.push_back(entity);
    }

    if(targetList.empty()) {
        shouldRotate = false;
        return;
    }

    std::sort(targetList.begin(), targetList.end(),
              [this](Actor* a1, Actor* a2) { return sortByDist(a1, a2); });

    Actor* target = targetList[0];
    if(!target)
        return;

    if(rotMode != 0 || strafe) {
        Vec3<float> targetPos = target->getEyePos();
        rotAngle = player->getEyePos().CalcAngle(targetPos);
        rotAngle5 = rotAngle;
        shouldRotate = true;
    } else {
        shouldRotate = false;
    }

    if(Counter(1000.0)) {
        Attack(target);

        if(enableHookSpeed) {
            LocalPlayer* lp = GI::getLocalPlayer();
            if(!lp || !lp->gamemode)
                return;

            for(int i = 0; i < hookAmount; i++) {
                lp->gamemode->attack(target);
                lp->swing();
            }

            for(int i = 0; i < multiplier - 1; i++) {
                lp->gamemode->attack(target);
                lp->swing();
            }

            if(autoHookSpeed)
                updateAutoHookSpeed();
        }
    }
}

void KillAuraH2::onUpdateRotation(LocalPlayer* player) {
    if(!player || !shouldRotate || targetList.empty())
        return;

    Actor* target = targetList[0];
    if(!target || !target->isAlive())
        return;

    auto* rot = player->getActorRotationComponent();
    auto* head = player->getActorHeadRotationComponent();
    auto* body = player->getMobBodyRotationComponent();
    if(!rot)
        return;

    switch(rotMode) {
        case 1:
        case 2:
            rot->mPitch = rotAngle5.x;
            rot->mYaw = rotAngle5.y;
            break;

        case 3: {
            auto* targetRot = target->getActorRotationComponent();
            if(!targetRot)
                break;
            float predictYaw = targetRot->mYaw + (headspeed - 90.0f);
            float rad = predictYaw * (M_PI / 180.0f);
            Vec3<float> tPos = target->getPos();
            Vec3<float> offset(-cos(rad) * test, 0, -sin(rad) * test);
            Vec3<float> predicted = tPos + offset;
            Vec2<float> angle = player->getPos().CalcAngle(predicted).normAngles();
            rot->mYaw = angle.y;
            rot->mPitch = angle.x;
            break;
        }

        default:
            return;
    }

    if(head)
        head->mHeadRot = rot->mYaw;
    if(body)
        body->yBodyRot = rot->mYaw;
}

void KillAuraH2::onLevelRender() {
    LocalPlayer* player = GI::getLocalPlayer();
    if(!player)
        return;

    auto* client = GI::getClientInstance();
    if(!client)
        return;

    auto* renderer = client->getLevelRenderer();
    if(!renderer || !renderer->getrenderplayer())
        return;

    Vec3<float> origin = renderer->getrenderplayer()->origin;

    if(visualTarget && !targetList.empty() && targetList[0])
        drawTargetHighlight(targetList[0], origin, targetColor);
}

void KillAuraH2::drawTargetHighlight(Actor* target, Vec3<float> origin, UIColor color) {
    if(!target)
        return;

    Vec3<float> targetPos = target->getPos() - origin;
    float radius = 0.7f;
    const int segments = 36;

    DrawUtil::setColor(color);
    DrawUtil::tessellator->begin(VertextFormat::LINE_STRIP, 2);

    for(int i = 0; i <= segments; i++) {
        float angle = (i * 2.0f * M_PI) / segments;
        float x = targetPos.x + radius * cosf(angle);
        float z = targetPos.z + radius * sinf(angle);
        DrawUtil::tessellator->vertex(x, targetPos.y, z);
    }

    MeshHelpers::renderMeshImmediately(DrawUtil::screenCtx, DrawUtil::tessellator,
                                       DrawUtil::blendMaterial);
}