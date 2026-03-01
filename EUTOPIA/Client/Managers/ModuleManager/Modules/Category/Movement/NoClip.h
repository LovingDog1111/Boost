#pragma once
#include "../../ModuleBase/Module.h"

class NoClip : public Module {
   private:
    float hSpeed = 1.f;
    float vSpeed = 0.5f;
    float Glide = -0.02f;

   public:
    NoClip();

    void onNormalTick(LocalPlayer* localPlayer) override;
    void onEnable() override;
    void onDisable() override;
};
