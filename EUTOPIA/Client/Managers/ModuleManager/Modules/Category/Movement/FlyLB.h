#pragma once
#include "../../ModuleBase/Module.h"

class FlyLB : public Module {
   private:
    bool smborder = false;
    bool bypass = false;
    bool dynamic = false;
    float hSpeed = 1.f;
    float vSpeed = 0.5f;
    float Glide = -0.02f;
    float currentSpeed = 1.f;

   public:
    FlyLB();

    void onNormalTick(LocalPlayer* localPlayer) override;
    void onSendPacket(Packet* packet);
    void onDisable() override;

    static float lerpfly(float endPoint, float current, float speed);
};
