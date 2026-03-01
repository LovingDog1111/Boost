#pragma once
#include "../../ModuleBase/Module.h"

class Fly : public Module {
   private:
    bool dynamic = false; 
    float hSpeed = 1.f;
    float currentSpeed = 1.f;
    float vSpeed = 0.5f;

   public:
    float Glide = -0.02f;
    Fly();

    virtual void onEnable() override;
    virtual void onDisable() override;

    void onNormalTick(LocalPlayer* localPlayer) override;
    void onSendPacket(Packet* packet);
};