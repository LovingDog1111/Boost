#pragma once
#include "../../ModuleBase/Module.h"

class SurfaceTP : public Module {
   public:
    SurfaceTP();

    void onEnable() override;
    void onNormalTick(LocalPlayer* localPlayer) override;
    void onDisable() override;

   private:
    Vec3<float> targetPos;
    bool lerping = false;
};