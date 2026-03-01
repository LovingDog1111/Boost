#pragma once
#include <Windows.h>

#include <vector>

#include "../../ModuleBase/Module.h"

class KillAura2 : public Module {
   private:
    float range = 5.f;
    float aps = 10.f;
    int interval = 1;
    int multiplier = 1;
    bool includeMobs = false;
    int rotMode = 0;
    bool shouldRotate = false;
    int strafeMode = 0;
    float strafeRadius = 5.0f;
    float strafeAngle = 0.0f;
    float rotationSpeed = 10.f;
    int timer = 20;
    int64_t lastAttackTime = 0;
    decltype(GetTickCount64()) start;
    std::vector<Actor*> targetList;
    int oTick = INT_MAX;
    Vec2<float> rotAngle;
    Vec2<float> rotAngle2;
    Vec2<float> ange;
    static bool sortByDist(Actor* a1, Actor* a2);
    int getBestWeaponSlot(Actor* target);
    void Attack(Actor* target);
    bool Counter(double a1);

   public:
    KillAura2();
    void onDisable() override;
    void onEnable() override;
    void onNormalTick(LocalPlayer* localPlayer) override;
    void onLevelRender() override;
    void onUpdateRotation(LocalPlayer* player);
};