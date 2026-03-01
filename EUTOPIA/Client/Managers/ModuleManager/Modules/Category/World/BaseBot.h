#pragma once
#include "../../../ModuleManager.h"
#include "../../ModuleBase/Module.h"
#include "../Player/PacketMine.h"

class BaseBot : public Module {
   private:
    int range = 80;
    int scanAttempts = 0;
    bool hopperSetting = false;
    bool enchantingTableSetting = false;
    bool bookshelfSetting = false;
    bool trappedChestSetting = false;
    bool enderChestSetting = false;
    bool anvilSetting = false;
    bool furnaceSetting = false;
    bool isPausedAtTarget = false;
    float tpDelay = 3.f;
    float tpTimer = 0.f;
    bool dbgmsgs = false;
    bool hasTarget = false;
    Vec3<float> targetPos;
    std::string currentActionMessage = "";

    bool isValidBlock(Block* blk);
    void resettimertodefault();
    void settimertofast();

   public:
    BaseBot();
    void onEnable() override;
    void onDisable() override;
    void onNormalTick(LocalPlayer* localPlayer) override;
};
