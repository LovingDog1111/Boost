#pragma once

#include "../../ModuleBase/Module.h"
#include "../../Utils/DrawUtil.h"
#include "../Client/Managers/ModuleManager/ModuleManager.h"
#include "../Client/Managers/ModuleManager/Modules/Category/Client/ColorsMod.h"
#include "PacketCounter.h"
#include "PlayerCounter.h"

class KillCounter : public Module {
   public:
    KillCounter();
    void onEnable() override;
    void onDisable() override;
    void onMCRender(MinecraftUIRenderContext* ctx) override;
    void onD2DRender() override;
    void updateTarget();

   private:
    int rendermode;
    int killCount = 0;
    Actor* target = nullptr;
    Actor* lastDeadTarget = nullptr;
};
