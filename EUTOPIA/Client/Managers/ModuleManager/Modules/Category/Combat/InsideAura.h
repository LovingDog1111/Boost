#pragma once
#include <chrono>
#include <vector>

#include "../../ModuleBase/Module.h"

class InsideAura : public Module {
   private:
    std::vector<Actor*> targetList;
    std::chrono::steady_clock::time_point lastAttack;
    std::int64_t lastAttackTime = 0;
    std::int64_t lastTeleportTime = 0;
    size_t switchIndex = 0;
    int mode = 0;
    float range = 5.f;
    bool includeMobs = false;
    int minCPS = 5;
    int maxCPS = 10;
    int delay = 0;
    int attackDelay = 0;
    float tpDistance = 1.f;
    float tpMoveSpeed = 1.f;
    float tpYOffset = 0.f;
    bool upAndDown = false;
    float upDownSpeed = 0.1f;
    float upDownRange = 1.f;
    float tpRandomness = 0.0f;

    float upDownOffset = 0.f;
    bool upDownGoingUp = true;

    void attack(Actor* target);
    Actor* getNextTarget();
    bool canAttack();

   public:
    InsideAura();
    virtual void onEnable() override;
    virtual void onDisable() override;
    virtual void onNormalTick(LocalPlayer* localPlayer) override;
};
