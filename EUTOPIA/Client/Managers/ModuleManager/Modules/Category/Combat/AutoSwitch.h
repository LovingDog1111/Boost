#pragma once
#include "../../ModuleBase/Module.h"

class AutoSwitch : public Module {
   private:
    int slot1 = 0;
    int slot2 = 1;
    float delay = 0.5f;
    float timer = 0.0f;
    bool toggle = false;

   public:
    AutoSwitch();
    void onNormalTick(LocalPlayer *player) override;
};
