#pragma once

#include "../../Utils/DrawUtil.h"
#include "../Client/Managers/ModuleManager/ModuleManager.h"
#include "../Client/Managers/ModuleManager/Modules/Category/Client/ColorsMod.h"
#include "../../ModuleBase/Module.h"

class TargetInfo : public Module {
   public:
    TargetInfo();
    void onMCRender(MinecraftUIRenderContext* ctx) override;
    void onD2DRender() override;

   private:
    float distance;
    bool isTargetValid();
    int rendermode;
    Actor* lastTarget = nullptr;
    Actor* target = nullptr;
    int hitCount = 0;  
    bool showhitcount;   

};

