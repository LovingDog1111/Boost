#include "AutoKill.h"

#include <FriendUtil.h>
#include <Minecraft/TargetUtil.h>
#include <Minecraft/WorldUtil.h>

#include <algorithm>
#include <cmath>

#include "../SDK/Render/MCR.h"
#include "../SDK/Render/MeshHelpers.h"
#include "../Utils/DrawUtil.h"

int hvhornormal = 0;

int akKillCount = 0;

AutoKill::AutoKill() : Module("AutoKill", "Auto Attack Targets", Category::COMBAT) {
    registerSetting(
        new EnumSetting("Mode", "change mode", {"Normal", "HvH"}, &hvhornormal, 0));
    registerSetting(new BoolSetting("EnableHookSpeed", "Enable hook speed", &enableHookSpeed, true));
        registerSetting(new BoolSetting("AutoHookSpeed", "Auto hook speed", &autoHookSpeed, false));

        registerSetting(
            new SliderSetting<int>("HookAmount", "Hook packets per attack", &hookAmount, 4, 1, 20));
        registerSetting(
            new SliderSetting<int>("Multiplier", "Extra packets per hook", &multiplier, 1, 1, 10));

        registerSetting(new EnumSetting("SpeedType", "Hook speed pattern",
                                        {"Fast", "Slow", "Medium", "Random"}, &speedType, 0));
        registerSetting(new EnumSetting("CfgMode", "Select mode", {"Lifeboat", "Cubecraft"}, &auramode, 0));
    registerSetting(new BoolSetting("AutoCfg", "Auto adapt settings", &autocfg, true));
    registerSetting(new SliderSetting<float>("Range", "Scan range", &akRange, 60.f, 3.f, 300));
    registerSetting(new SliderSetting<float>("Speed", "Follow speed", &akTpSpeed, 0.5f, 0.1f, 3.f));
    registerSetting(
        new SliderSetting<float>("AttackSpeed", "APS", &akAttackSpeed, 6.f, 0.1f, 20.f));
    registerSetting(
        new SliderSetting<float>("Y Offset", "Height above target", &akYOffset, 1.5f, 0.f, 10.f));
    registerSetting(new BoolSetting("Include Mobs", "Include mobs", &akIncludeMobs, false));
    registerSetting(new BoolSetting("Swing", "Swing weapon", &akSwing, true));
}

void AutoKill::setTimerDefault() {
    if(auto ci = GI::getClientInstance()) {
        if(auto mc = ci->minecraftSim) {
            mc->setSimTimer(20.f);
            mc->setSimSpeed(20.f / 20.f);
        }
    }
}


void fastboi() {
    if(auto ci = GI::getClientInstance()) {
        if(auto mc = ci->minecraftSim) {
            mc->setSimTimer(100.f);
            mc->setSimSpeed(100.f/ 20.f);
        }
    }
}

void AutoKill::updateAutoHookSpeed() {
    autoSpeedCounter++;

    switch(speedType) {
        case 0:  
            hookAmount = 6 + (autoSpeedCounter % 5);
            break;

        case 1:  
            hookAmount = 2 + (autoSpeedCounter % 4);
            break;

        case 2:
            hookAmount = 5 + (autoSpeedCounter % 3);
            break;

        case 3:  
            hookAmount = rand() % 15 + 1;
            break;
    }

    if(autoSpeedCounter > 1000)
        autoSpeedCounter = 0;
}


bool AutoKill::sortDist(Actor* a, Actor* b) {
    Vec3<float> p = Game.getLocalPlayer()->getPos();
    float da = std::sqrt((a->getPos().x - p.x) * (a->getPos().x - p.x) +
                         (a->getPos().y - p.y) * (a->getPos().y - p.y) +
                         (a->getPos().z - p.z) * (a->getPos().z - p.z));
    float db = std::sqrt((b->getPos().x - p.x) * (b->getPos().x - p.x) +
                         (b->getPos().y - p.y) * (b->getPos().y - p.y) +
                         (b->getPos().z - p.z) * (b->getPos().z - p.z));
    return da < db;
}


void AutoKill::onEnable() {
    setTimerDefault();
    akLocked = nullptr;
    akMessageSent = false;
    akLastAtk = std::chrono::steady_clock::now();
}

void AutoKill::resetAABB(LocalPlayer* p) {
    if(!p)
        return;
    AABB a = p->getAABB(true);
    a.upper.y = a.lower.y + 1.8f;
    p->setAABB(a);
}

void AutoKill::onDisable() {
    setTimerDefault();
    resetAABB(Game.getLocalPlayer());
    akKillCount = 0;
    GI::DisplayClientMessage("[AutoKill] Disabled");
}

void AutoKill::performNormalHook(LocalPlayer* p, Actor* target) {

    Vec3<float> targetPos = target->getAABB(true).getCenter();
    targetPos.y += akYOffset;

    Vec3<float> predicted = targetPos;
    Vec3<float> vel = target->getStateVectorComponent()->mVelocity;
    predicted.x += vel.x * 0.2f;
    predicted.y += vel.y * 0.2f;
    predicted.z += vel.z * 0.2f;

    Vec3<float> pos = p->getPos();
    float dx = predicted.x - pos.x;
    float dy = predicted.y - pos.y;
    float dz = predicted.z - pos.z;

    float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
    if(dist > 0.01f) {
        float r = akTpSpeed / std::max(dist, 0.01f);
        Vec3<float> move{dx * r, dy * r, dz * r};
        p->lerpMotion(move);

        AABB a = p->getAABB(true);
        a.lower = a.lower.add(move);
        a.upper = a.upper.add(move);
        p->setAABB(a);
    }

    auto now = std::chrono::steady_clock::now();
    float atkMs = 1000.f / akAttackSpeed;

    if(std::chrono::duration_cast<std::chrono::milliseconds>(now - akLastAtk).count() >= atkMs) {
        p->getgamemode()->attack(target);
        if(akSwing)
            p->swing();

        if(enableHookSpeed) {
            for(int i = 0; i < hookAmount; i++) {
                p->getgamemode()->attack(target);
                if(akSwing)
                    p->swing();
            }
            for(int i = 0; i < multiplier; i++) {
                p->getgamemode()->attack(target);
                if(akSwing)
                    p->swing();
            }
            if(autoHookSpeed)
                updateAutoHookSpeed();
        }

        akLastAtk = now;
    }

    /*
    float yaw = std::atan2(dz, dx) * 180.f / 3.14159265f - 90.f;
    float pitch = -std::atan2(dy, std::sqrt(dx * dx + dz * dz)) * 180.f / 3.14159265f;

    auto* rot = p->getActorRotationComponent();
    auto* head = p->getActorHeadRotationComponent();
    auto* body = p->getMobBodyRotationComponent();

    if(rot) {
        rot->mYaw = yaw;
        rot->mPitch = pitch;
    }
    if(head)
        head->mHeadRot = yaw;
    if(body)
        body->yBodyRot = yaw;
        */
}

void AutoKill::performHookAura(LocalPlayer* p, Actor* target) {
    if(!target || !p)
        return;

    if(hvhornormal == 0) {
        performNormalHook(p, target);
        return;
    }

    Vec3<float> targetPos = target->getAABB(true).getCenter();
    targetPos.y += akYOffset;

    Vec3<float> predicted = targetPos;
    Vec3<float> vel = target->getStateVectorComponent()->mVelocity;
    predicted.x += vel.x * 0.1f;
    predicted.y += vel.y * 0.1f;
    predicted.z += vel.z * 0.1f;

    Vec3<float> pos = p->getPos();
    Vec3<float> move = predicted - pos;

    float dist = std::sqrt(move.x * move.x + move.y * move.y + move.z * move.z);
    if(dist > 7.f) {
        float scale = 7.f / dist;
        move.x *= scale;
        move.y *= scale;
        move.z *= scale;
    }

    p->lerpMotion(move);
    AABB a = p->getAABB(true);
    a.lower = a.lower.add(move);
    a.upper = a.upper.add(move);
    p->setAABB(a);

    std::vector<Vec3<float>> offsets = {
        {0.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, -1.f, 0.f}, {1.f, 0.f, 0.f}, {-1.f, 0.f, 0.f}};
    auto now = std::chrono::steady_clock::now();
    float atkMs = 1000.f / akAttackSpeed;

    if(std::chrono::duration_cast<std::chrono::milliseconds>(now - akLastAtk).count() >= atkMs) {
        for(auto& off : offsets) {
            p->getgamemode()->attack(target);
            if(akSwing)
                p->swing();

            if(enableHookSpeed) {
                for(int i = 0; i < hookAmount; i++) {
                    p->getgamemode()->attack(target);
                    if(akSwing)
                        p->swing();
                }
                for(int i = 0; i < multiplier; i++) {
                    p->getgamemode()->attack(target);
                    if(akSwing)
                        p->swing();
                }
                if(autoHookSpeed)
                    updateAutoHookSpeed();
            }
        }
        akLastAtk = now;
    }
}




void AutoKill::onNormalTick(LocalPlayer* p) {
    if(!p || !p->level)
        return;

    fastboi();

    if(autocfg) {
        if(auramode == 0) {
            akRange = 300.f;
            akTpSpeed = 3.f;
            akYOffset = 6.5f;
            akAttackSpeed = 20.f;
        }
        if(auramode == 1) {
            akRange = 10.f;
            akTpSpeed = 1.f;
            akYOffset = 4.5f;
            akAttackSpeed = 10.f;
        }
    }

    if(akLocked) {
        Vec3<float> pos = p->getPos();
        Vec3<float> targetPos = akLocked->getAABB(true).getCenter();
        targetPos.y += akYOffset;

        float dx = targetPos.x - pos.x;
        float dy = targetPos.y - pos.y;
        float dz = targetPos.z - pos.z;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

     

        bool exists = false;
        std::vector<Actor*> actors = p->level->getRuntimeActorList(); 
        for(auto* e : actors) {
            if(e == akLocked) {
                exists = true;
                break;
            }
            if(!exists || !akLocked->isAlive() || akLocked->getHealth() <= 0) {
                akKillCount++;
                if(akLocked != nullptr && !akLocked->getNameTag().empty()) {
                    GI::DisplayClientMessage(
                        ("[AutoKill] Killed " + akLocked->getNameTag()).c_str());
                } else {
                    GI::DisplayClientMessage("[AutoKill] Killed Unknown");
                }
            


                setTimerDefault();

                akLocked = nullptr;
                akMessageSent = false;
            }
        }
    } else {
        setTimerDefault();
    }

    if(!akLocked) {
        akTargets.clear();
        for(auto& e : p->level->getRuntimeActorList()) {
            if(!TargetUtil::isTargetValid(e, akIncludeMobs))
                continue;
            if(FriendManager::isFriend(e->getNameTag()))
                continue;
            if(p->getPos().dist(e->getPos()) <= akRange)
                akTargets.push_back(e);
        }

        if(akTargets.empty()) {
            if(akKillCount > 0) {

                setTimerDefault();

                GI::DisplayClientMessage((std::string("[AutoKill] Completed, killed ") +
                                          std::to_string(akKillCount) + " players")
                                             .c_str());
                akKillCount = 0;
            }
            GI::DisplayClientMessage("[AutoKill] No targets found");

            setTimerDefault();

            this->setEnabled(false);
            return;
        }

        std::sort(akTargets.begin(), akTargets.end(), sortDist);
        akLocked = akTargets[0];
        akMessageSent = false;
    }

    if(!akMessageSent) {
        GI::DisplayClientMessage(("[AutoKill] Locked onto " + akLocked->getNameTag()).c_str());
        GI::DisplayClientMessage(("[AutoKill] " + akLocked->getNameTag() +
                                  "'s health: " + std::to_string(akLocked->getHealth()))
                                     .c_str());

        akMessageSent = true;
    }
    setTimerDefault();
    performHookAura(p, akLocked);
}


void AutoKill::onLevelRender() {
    LocalPlayer* localPlayer = Game.getLocalPlayer();
    if(!localPlayer)
        return;
    Tessellator* tessellator = DrawUtil::tessellator;
    if(!tessellator)
        return;

    Vec3<float> origin = Game.getClientInstance()->getLevelRenderer()->renderplayer->origin;
    Vec3<float> start{0.0f, 0.0f, 0.0f};
    DrawUtil::setColor(UIColor(255, 255, 255, 175));

    Actor* target = akLocked;
    if(target && target->isAlive()) {
        Vec3<float> end = target->getEyePos().sub(origin);
        tessellator->begin(VertextFormat::LINE_LIST, 2);
        tessellator->vertex(start.x, start.y, start.z);
        tessellator->vertex(end.x, end.y, end.z);
        MeshHelpers::renderMeshImmediately(DrawUtil::screenCtx, tessellator,
                                           DrawUtil::blendMaterial);
    }
}
