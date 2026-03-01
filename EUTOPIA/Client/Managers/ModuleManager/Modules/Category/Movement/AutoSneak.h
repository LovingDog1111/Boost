#pragma once

#include "../../ModuleBase/Module.h"

class AutoSneak : public Module {
   public:
    AutoSneak();
    virtual void onDisable() override;
    void onNormalTick(LocalPlayer* localPlayer) override;
};
