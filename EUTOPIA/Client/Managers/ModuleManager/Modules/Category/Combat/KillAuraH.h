#pragma once
#include <Windows.h>

#include <chrono>
#include <vector>

#include "../../ModuleBase/Module.h"

class KillAuraH2 : public Module {
   private:
    float range = 5.f;
    float aps = 100.f;
    bool enableHookSpeed = true;
    bool autoHookSpeed = false;
    int hookAmount = 5;
    int multiplier = 1;
    int speedType = 0;
    int rotMode = 0;
    bool shouldRotate = false;
    Vec2<float> rotAngle;
    Vec2<float> rotAngle5;
    bool strafe = false;
    float test = 0.f;
    float headspeed = 30.f;
    bool includeMobs = false;
    bool visualTarget = false;
    bool visualRange = false;
    UIColor targetColor = UIColor(255, 0, 0, 255);
    int attackCounter = 0;
    int hookCounter = 0;
    int autoSpeedCounter = 0;
    DWORD start = 0;
    DWORD lastAttackTime = 0;
    std::vector<Actor*> targetList;

    bool sortByDist(Actor* a1, Actor* a2);
    void Attack(Actor* target);
    bool Counter(double a1);
    void updateAutoHookSpeed();
    void drawTargetHighlight(Actor* target, Vec3<float> origin, UIColor color);

   public:
    KillAuraH2();
    void onEnable() override;
    void onDisable() override;
    void onNormalTick(LocalPlayer* player) override;
    void onUpdateRotation(LocalPlayer* player) override;
    void onLevelRender() override;
};