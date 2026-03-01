#pragma once
#pragma once
#include "../../ModuleBase/Module.h"

class Timer : public Module {
   private:
    int timerValue = 20;
    float prevTimer = 20.0f;

   public:
    Timer();

    void onEnable() override;
    void onDisable() override;
    void onNormalTick(LocalPlayer* localPlayer) override;
};