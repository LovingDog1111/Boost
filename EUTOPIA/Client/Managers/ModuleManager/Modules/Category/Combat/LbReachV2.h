#pragma once


#include "../../ModuleBase/Module.h"

class LbReachV2 : public Module {
   private:
    float longReach = 50.0f;
    int hitmode = 0;

    void sendPos(Vec3<float> p);

   public:
    LbReachV2();

    static bool isTeleporting;

    virtual void onEnable() override;
    virtual void onLevelRender() override;
    virtual void onDisable() override;
};