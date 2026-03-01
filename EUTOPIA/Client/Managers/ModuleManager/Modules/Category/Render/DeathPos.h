#pragma once
#include <string>
#include <vector>

#include "../../ModuleBase/Module.h"

struct DeathPosition {
    Vec3<float> pos;
    int dimensionId;
};

class DeathPos : public Module {
   private:
    std::vector<DeathPosition> deathPositions;
    float coordSize = 1.0f;
    bool lbMode = false;
    float lbHealthThreshold = 10.0f;
    bool resetFlag = false;
    bool syncColor = false;
    void resetDeaths();

   public:
    DeathPos();

    void onEnable() override;
    void onDisable() override;
    void onNormalTick(LocalPlayer* localPlayer) override;
    void onD2DRender() override;
};