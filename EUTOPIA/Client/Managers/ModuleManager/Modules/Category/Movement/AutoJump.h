#pragma once
#include "../../ModuleBase/Module.h"

class AutoJump : public Module {
   public:
    AutoJump();

    virtual void onDisable() override;

    void onNormalTick(LocalPlayer* localPlayer) override;
};
