#pragma once
#include "../../ModuleBase/Module.h"

class KillAura : public Module {
   public:
    enum class AttackMode { Vanilla, IDPredict };

    KillAura();

    void onNormalTick(LocalPlayer* lp);
    std::vector<Actor*> getTargets(LocalPlayer* lp);
    std::string getModeText() override;
    void onLevelRender() override;
    void onDisable() override;

   private:
    float range;
    bool mobs;
    int attackDelay;
    int lastAttackTick;
    AttackMode mode;
    int hitAttempts;
    bool eatStop;
    bool render;
    UIColor renderColor;
    std::vector<Actor*> targetList;
    int autoWeaponMode;

    void attackTarget(LocalPlayer* lp, Actor* target);
    int getBestWeaponSlot(Actor* target);
};