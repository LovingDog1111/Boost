#pragma once
#include "../../ModuleBase/Module.h"


class Keystrokes : public Module {
   private:
    float scale = 1.0f;
    int opacity = 160;

   public:
    Keystrokes();
    ~Keystrokes();

    void onD2DRender() override;
};