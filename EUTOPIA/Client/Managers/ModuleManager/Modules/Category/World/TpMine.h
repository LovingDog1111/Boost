#pragma once
#include "../../ModuleBase/Module.h"

class TpMine : public Module {
   public:
    bool mineDiamond = true;
    bool mineIron = false;
    bool mineGold = false;
    bool mineCoal = false;
    bool mineLapis = false;
    bool mineRedstone = false;
    bool mineEmerald = false;
    bool mineQuartz = false;
    bool mineAncient = false;
    int range = 80;

    bool hasTarget = false;
    Vec3<float> targetPos = Vec3<float>(0, 0, 0);

    TpMine();
    void onEnable() override;
    void onNormalTick(LocalPlayer* player) override;
};