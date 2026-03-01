#pragma once



#include "../../ModuleBase/Module.h"
#include "../../Utils/TimerUtil.h"

class DashAura : public Module {
   private:
    std::vector<Actor*> targetList;
    std::chrono::steady_clock::time_point lastAttack;
    std::int64_t lastAttackTime = 0;
    std::int64_t lastTeleportTime = 0;

    float range = 5.f;
    int minCPS = 5;
    int maxCPS = 10;
    int delay = 0;
    int attackDelay = 0;
    float tpDistance = 1.f;
    float tpMoveSpeed = 1.f;
    float tpYOffset = 0.f;
    bool throughWallz = false;

    Actor* getNextTarget();
    void attack(Actor* target);
    bool canAttack();

   public:
    DashAura();
    virtual void onEnable() override;
    virtual void onDisable() override;
    virtual void onNormalTick(LocalPlayer* localPlayer) override;
};
