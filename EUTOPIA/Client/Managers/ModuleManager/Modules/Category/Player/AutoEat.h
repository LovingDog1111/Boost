#pragma once
#include "../../ModuleBase/Module.h"

class AutoEat : public Module {
   private:
    int minFoodCount;
    int delayy;
    int tickCounter;
    int minHealth;
    bool autoEatt;

   public:
    AutoEat();
    bool eating = false;
    bool isEating() const {
        return eating;
    }

    void onNormalTick(LocalPlayer* localPlayer) override;
    void onEnable() override;   
    void onDisable() override;
};
