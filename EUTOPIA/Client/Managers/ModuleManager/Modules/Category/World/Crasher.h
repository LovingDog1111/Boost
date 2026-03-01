#pragma once
#include <string>

#include "../../../ModuleManager.h"
#include "../../ModuleBase/Module.h"

class Crasher : public Module {
   public:
    enum class ModeType { idk, BDS };

    Crasher();

    void onNormalTick(LocalPlayer* localPlayer) override;



   private:
    ModeType mode = ModeType::idk;
    SliderSetting<int>* packetRate = nullptr;
    BoolSetting* autoCrash = nullptr;
};
