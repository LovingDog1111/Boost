#pragma once
#include "../../ModuleBase/Module.h"

class NoCameraClip : public Module {
   public:
    NoCameraClip();
    void onEnable() override;
    void onDisable() override;


};
