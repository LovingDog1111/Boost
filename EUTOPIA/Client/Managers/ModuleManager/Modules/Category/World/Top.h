#pragma once
#include "../../ModuleBase/Module.h"

class Top : public Module {
   public:
    Vec3<float> goal;
    Vec3<float> startPos;
    bool works = false;
    int ticks = 0;

    Top();
    void onEnable() override;
    void onNormalTick(LocalPlayer* localPlayer) override;
    void onDisable() override;
};
