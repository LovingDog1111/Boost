#pragma once

#include <chrono>
#include <vector>

#include "../../ModuleBase/Module.h"

class InfinityAura : public Module {
   public:
    InfinityAura();

    void onEnable() override;
    void onDisable() override;
    void onNormalTick(LocalPlayer* player) override;
    void onUpdateRotation(LocalPlayer* player) override;
    void onSendPacket(Packet* packet) override;

   private:
    enum MovementState { ToTarget, Returning };

    float range = 5.f;
    float wallRange = 3.f;
    bool includeMobs = false;

    float attackSpeed = 5.f;
    float swingSpeed = 5.f;
    int multiplier = 1;
    float tpSpeed = 1.f;

    bool shallReturn = false;
    bool resetOnAttack = false;

    int rotMode = 1;
    bool bodyYaw = false;
    bool doStrafing = true;
    bool rayTraceBypass = false;
    int pitchOffset = 0;

    std::vector<Actor*> targetList;
    Vec2<float> rotAngle;

    bool shouldRotate = false;
    MovementState moveState = ToTarget;

    Vec3<float> origin;
    Vec3<float> spoofedPos;
    int step = 1;

    std::chrono::steady_clock::time_point lastAttackTime = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point lastSwingTime = std::chrono::steady_clock::now();

    static bool sortByDist(Actor* a, Actor* b);
};
