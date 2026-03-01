#pragma once
#include "../../ModuleBase/Module.h"

class Switcher : public Module {
   private:
   public:
    Switcher();


    virtual void onNormalTick(LocalPlayer* localPlayer) override;
};
