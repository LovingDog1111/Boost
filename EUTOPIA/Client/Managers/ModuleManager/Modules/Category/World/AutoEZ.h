#pragma once

#include "../../ModuleBase/Module.h"

class AutoEZ : public Module {
   public:
    AutoEZ();


    virtual void onNormalTick(LocalPlayer* actor) override;
};