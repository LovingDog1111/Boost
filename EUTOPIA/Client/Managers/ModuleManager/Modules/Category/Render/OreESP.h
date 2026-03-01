#pragma once
#include <chrono>
#include <unordered_map>

#include "../../ModuleBase/Module.h"

class OreESP : public Module {
   public:
    std::unordered_map<BlockPos, UIColor> blockList;

    bool diamondOre;
    bool goldOre;
    bool ironOre;
    bool coalOre;
    bool redstoneOre;
    bool lapisOre;
    bool emeraldOre;
    bool quartzOre;

    int radius;
    int alpha;
    int lineAlpha;

    int updateDelayMs;
    std::chrono::steady_clock::time_point lastUpdate;

    OreESP();

    void onLevelRender() override;
    void updateBlockList(BlockSource* source, const BlockPos& pos);
};
