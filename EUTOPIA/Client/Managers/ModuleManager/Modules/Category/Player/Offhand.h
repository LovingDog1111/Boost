#pragma once
#pragma once
#include "../../ModuleBase/Module.h"
#include "../../ModuleBase/Settings/BoolSetting.h"
#include "../../ModuleBase/Settings/SliderSetting.h"

class Offhand : public Module {
   private:
    int itemMode = 0;
    EnumSetting* itemModeEnum = nullptr;

    SliderSetting<int>* healthThresholdSetting = nullptr;
    BoolSetting* autoTotemSwitchSetting = nullptr;

    bool lastWasTotem = false;

   public:
    Offhand();
    std::string getModeText() override;
    void onNormalTick(LocalPlayer* localPlayer) override;
};