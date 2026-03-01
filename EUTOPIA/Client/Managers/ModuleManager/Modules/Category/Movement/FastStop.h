#pragma once
#include "../../ModuleBase/Module.h"

class FastStop : public Module {
   public:
    FastStop();


    void onNormalTick(LocalPlayer* localPlayer) override;

};
