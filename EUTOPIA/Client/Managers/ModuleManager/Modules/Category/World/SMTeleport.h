#pragma once
#include "../../ModuleBase/Module.h"

class SMTeleport : public Module {
   private:
    float speed = 1.5f;
    float arrivalThreshold = 1.0f;
    float timerValue = 725000.f;
    float prevTimer = 20.f;
    bool hasTarget = false;
    Vec3<float> targetPos = Vec3<float>(0.f, 0.f, 0.f);
    void resetAABB(LocalPlayer* player);


   public:
    SMTeleport();
    void onEnable() override;
    void onDisable() override;
    void onNormalTick(LocalPlayer* localPlayer) override;
    void setTargetPos(Vec3<float> pos);
};
