#pragma once

#include "../../ModuleBase/Module.h"

class Twerk : public Module {
   public:
    Twerk();
    void onNormalTick(LocalPlayer* localPlayer) override;
    void onDisable() override;

   private:
    bool sneakState = false;
};
