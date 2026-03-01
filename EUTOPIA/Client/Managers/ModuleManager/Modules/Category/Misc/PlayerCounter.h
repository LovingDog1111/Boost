#pragma once
#include "../../ModuleBase/Module.h"

class PlayerCounter : public Module {
   private:
    int playerCount = 0;
    int rendermode;

   public:
    PlayerCounter();

    void onMCRender(MinecraftUIRenderContext* ctx) override;
    void onD2DRender() override;
};
