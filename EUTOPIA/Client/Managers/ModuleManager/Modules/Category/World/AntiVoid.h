#pragma once
#include "../../ModuleBase/Module.h"




class AntiVoid : public Module {
   private:

    std::vector<Vec3<float>> mOnGroundPositions;
    bool mTeleported = false;
    bool mCanTeleport = true;
    bool mTeleport = true;
    bool mTpOnce = false;
    float mFallDistance = 5.0f;



   public:
   AntiVoid();
    void onEnable() override;
   void onNormalTick(LocalPlayer* localPlayer) override;
};