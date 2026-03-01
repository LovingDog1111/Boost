#pragma once

#include <chrono>
#include <vector>


#include "../../ModuleBase/Module.h"


class AboveAura : public Module {
   public:
    AboveAura();
    void onEnable() override;
    void onDisable() override;
    void onNormalTick(LocalPlayer* player) override;
    void onSendPacket(Packet* packet) override;
    void onUpdateRotation(LocalPlayer* player) override;

   private:
    std::vector<Actor*> targetList;
    Vec2<float> rotAngle;
    bool shouldRotate = false;

    bool includeMobs = false;
    bool shallReturn = false;
    bool resetOnAttack = false;
    bool upAndDown = false;
    bool upDownGoingUp = true;

    float attackSpeed = 5.f;
    float tpSpeed = 1.f;
    float range = 5.f;
    float yOffset = 1.f;
    float upDownSpeed = 0.1f;
    float upDownRange = 1.f;
    float upDownOffset = 0.f;

    int step = 1;

    enum MoveState { ToTarget, Returning };
    MoveState moveState = ToTarget;

    Vec3<float> origin;

    std::chrono::steady_clock::time_point lastAttackTime;
    std::chrono::steady_clock::time_point lastTeleportTime;
};
