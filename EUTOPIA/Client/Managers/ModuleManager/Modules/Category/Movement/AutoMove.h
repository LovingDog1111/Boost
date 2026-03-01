#pragma once

#include "../../../ModuleManager.h"
#include "../../ModuleBase/Module.h"


struct LocalPlayer;

class AutoMove : public Module {
   public:
    AutoMove();
    void onEnable();
    void onDisable();
    void onNormalTick(LocalPlayer* localPlayer);


   private:
    float speed;
};
