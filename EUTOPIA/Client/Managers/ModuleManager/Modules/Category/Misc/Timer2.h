#pragma once
#include "../../ModuleBase/Module.h"

class Timer2 : public Module {
   private:
    int timerValue = 40;

   public:
    Timer2();

    void onDisable() override;
    void onClientTick() override;
};