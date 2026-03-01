#pragma once
#include "../../ModuleBase/Module.h"


class AutoHub : public Module {
   public:
    AutoHub();


    void onNormalTick(LocalPlayer* localPlayer) override;
    virtual void onEnable() override;
    virtual void onDisable() override;

   private:
    int healthThreshold = 6;
    bool alreadyTeleported = false;

    void sendHub();
};
