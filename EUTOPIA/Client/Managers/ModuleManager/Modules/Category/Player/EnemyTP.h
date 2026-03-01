#pragma once
#include "../../ModuleBase/Module.h"

class EnemyTP : public Module {
   private:
    int range = 50;

    static std::vector<Actor*> targetListTP;
    static bool works;
    static bool tpdone;
    static float closestDistance;
    static Vec3<float> goal;
    static int tickCounter;
    void resetAABB(LocalPlayer* player);
    void setDefaultTimer();
    void setTimerFast();

   public:
    EnemyTP();

    static bool sortByDist(Actor* a1, Actor* a2);

    virtual void onEnable() override;
    virtual void onNormalTick(LocalPlayer* actor) override;
    virtual void onDisable() override;
};