#pragma once
#include "../../ModuleBase/Module.h"

class DihESP : public Module {
   private:
    int alpha = 200;
    float speed = 2.0f;

   public:
    DihESP();
    void onLevelRender() override;
};
