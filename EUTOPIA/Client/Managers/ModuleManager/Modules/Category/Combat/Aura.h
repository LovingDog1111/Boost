#pragma once
#include <Windows.h>

#include <vector>

#include "../../ModuleBase/Module.h"

class KillAura3 : public Module {
   private:
    float range = 5.f;
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
    float boostAmount = 2.0f;
    float boostDelay = 5.0f;
    bool isBoosting = false;
    int boostTicks = 0;
    int lastBoostTime = 0;
    float boostTimer = 0.f;
    bool boost = false;
    bool attacking12 = false;
    int methodFE = 0;
    int cps = 0;
    int cpstest = 20;
    static bool sortByDist(Actor* a1, Actor* a2);
    int getBestWeaponSlot(Actor* target);
    int bestslot(Actor* target);
    void Attack(Actor* target);
    bool Counter(double a1);

   public:
    KillAura3();
    void onDisable() override;
    void onEnable() override;
    void onNormalTick(LocalPlayer* localPlayer) override;
    void onLevelRender() override;
    void onSendPacket(Packet* packet) override;
    void onUpdateRotation(LocalPlayer* localPlayer);
};