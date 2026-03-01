#pragma once
#include <chrono>
#include <vector>

#include "../../../ModuleManager.h"
#include "../../ModuleBase/Module.h"

class AutoKill : public Module {
   public:
    AutoKill();

    void onEnable() override;
    void onDisable() override;
    void onNormalTick(LocalPlayer* p) override;
    void onLevelRender() override;

   private:
    void performNormalHook(LocalPlayer* p, Actor* target);
    void setTimerDefault();
    void resetAABB(LocalPlayer* p);
    void performHookAura(LocalPlayer* p, Actor* target);
    static bool sortDist(Actor* a, Actor* b);
    void updateAutoHookSpeed();

    bool enableHookSpeed = true;
    bool autoHookSpeed = false;
    int hookAmount = 4;
    int multiplier = 1;
    int speedType = 0;
    int autoSpeedCounter = 0;

    static inline std::vector<Actor*> akTargets;
    static inline Actor* akLocked = nullptr;
    static inline bool akMessageSent = false;
    static inline std::chrono::steady_clock::time_point akLastAtk;

    static inline float akRange = 60.f;
    static inline float akTpSpeed = 0.5f;
    static inline float akYOffset = 1.5f;
    static inline float akAttackSpeed = 6.f;
    static inline bool akIncludeMobs = false;
    static inline bool akSwing = true;

    static inline bool autocfg = true;
    static inline int auramode = 0;
};
