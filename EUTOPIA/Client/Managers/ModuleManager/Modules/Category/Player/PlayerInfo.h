#pragma once

#include "../../ModuleBase/Module.h"

class PlayerInfo : public Module {
   public:
    PlayerInfo();
    void onNormalTick(LocalPlayer* player) override;

   private:
    bool wasDown = false;
};
