#pragma once
#include "../../ModuleBase/Module.h"

class ChestTP : public Module {
   private:
    int range = 300;
    bool lbMode = false;
    bool skipDoubleChests = false;
    bool lerping = false;
    Vec3<float> targetPos;

   public:
    ChestTP();

    void onEnable() override;
    void onNormalTick(LocalPlayer* player) override;
    void onDisable() override;

    void setLBMode(bool enabled) {
        lbMode = enabled;
    }
    void setSkipDoubleChests(bool enabled) {
        skipDoubleChests = enabled;
    }
    void setRange(int r) {
        range = r;
    }
};
