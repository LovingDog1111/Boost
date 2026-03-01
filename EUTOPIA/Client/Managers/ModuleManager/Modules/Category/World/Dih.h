#pragma once
#include "../../ModuleBase/Module.h"

class Dih : public Module {
   private:
    bool towerMode = false;
    bool airPlace = false;
    float mAnimBlockCount = 0.f;

   public:
    Dih();
    void onNormalTick(LocalPlayer* localPlayer) override;

   private:
    bool canPlaceBlock(const BlockPos& pos);
    int selectBlockSlot(LocalPlayer* player);
    void updateHUDAnimation(float deltaTime);
    Vec3<float> lastPos;
    float buildTimer;

};
