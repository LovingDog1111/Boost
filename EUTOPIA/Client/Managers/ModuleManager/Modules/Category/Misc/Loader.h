#pragma once
#include <vector>

#include "../../ModuleBase/Module.h"


class Loader : public Module {
   public:
    Loader();

    void onEnable() override;
    void onDisable() override;
    void onNormalTick(LocalPlayer* localPlayer) override;

   private:
    std::vector<Actor*> entityList;
};
