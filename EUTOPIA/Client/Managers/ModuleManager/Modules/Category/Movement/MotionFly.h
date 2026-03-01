#pragma once
#include "../../ModuleBase/Module.h"

class MotionFly : public Module {
   private:
    float hSpeed = 1.f;
    float vSpeed = 0.5f;
    float Glide = 0.f;
    bool timerBoost = false;
    float timerBoostValue = 1.f;
    bool applyGlideFlags = false;

   public:
    MotionFly();
    virtual void onEnable() override;
    virtual void onDisable() override;
    void onNormalTick(LocalPlayer* localPlayer) override;
    void onSendPacket(Packet* packet) override;
};
