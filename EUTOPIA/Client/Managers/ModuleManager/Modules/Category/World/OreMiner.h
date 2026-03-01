#pragma once
#include "../../../ModuleManager.h"
#include "../../ModuleBase/Module.h"
#include "../Player/PacketMine.h"

class OreMiner : public Module {
   public:
    OreMiner();
    void onEnable() override;
    void onNormalTick(LocalPlayer*) override;
    void onUpdateRotation(LocalPlayer*) override;
    void onDisable() override;
    int orerange = 3;
    bool itemCollected = false;
    void breakBlock30(LocalPlayer* localPlayer, GameMode* gm, PacketMine* packetMine,
                    BlockPos targetPos);
    BlockPos targetBlock;



   private:
    void breakBlock(LocalPlayer* localPlayer, GameMode* gm, PacketMine* packetMine,
                    BlockPos targetPos);

    void mineAboveDrop(LocalPlayer* lp, GameMode* gm, PacketMine* pm, BlockSource* region,
                       BlockPos bp);

    int range = 3;
    bool diamond = true;
    bool iron = false;
    bool gold = false;
    bool coal = false;
    bool redstone = false;
    bool emerald = false;
    bool ancient = false;
    bool breakPlayerBlocks = true;
};