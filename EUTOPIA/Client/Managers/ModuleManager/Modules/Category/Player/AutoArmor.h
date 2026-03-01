#pragma once
#include "../../ModuleBase/Module.h"

class AutoArmor : public Module {
   public:
    AutoArmor();
    void onNormalTick(LocalPlayer* lp) override;

   private:
    uint64_t lastAction = 0;
    int delay = 80;
};
