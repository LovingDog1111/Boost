#pragma once
#include <vector>

#include "../../ModuleBase/Module.h"


class MobTP : public Module {
   public:
    static bool tpCows;
    static bool tpSheeps;
    static bool tpCreepers;
    static bool tpZombies;
    static bool tpSkeletons;
    static bool tpSpiders;

    static std::vector<Actor*> targetList;
    static Vec3<float> goal;
    static bool works;
    static bool tpDone;
    static float closestDistance;
    static float speed;
    static float arrivalThreshold;
    static float prevTimer;

    MobTP();

    void onEnable() override;
    void onNormalTick(LocalPlayer* player) override;
    void onDisable() override;

   private:
    bool sortByDist(Actor* a1, Actor* a2);
    void resetAABB(LocalPlayer* player);
    void setTimerFast();
    void setDefaultTimer();
};
