#pragma once
#include "../../ModuleBase/Module.h"

class AutoPickaxe : public Module {
   private:
   public:
    AutoPickaxe();

    virtual void onNormalTick(LocalPlayer* localPlayer) override;
};