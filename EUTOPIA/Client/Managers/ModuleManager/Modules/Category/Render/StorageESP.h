#pragma once
#include <unordered_map>

#include "../../ModuleBase/Module.h"

class StorageESP : public Module {
   public:
    std::unordered_map<BlockPos, UIColor> blockList;
    bool chest;
    bool enderChest;
    bool shulkerBox;
    bool barrel;
    bool hopper;
    bool furnace;
    bool spawner;
    bool bookshelf;
    bool enchantingTable;
    bool itemFrame;
    bool decoratedPot;
    bool web;
    bool dropper;
    bool dispenser;
    bool anvil;
    int radius;
    int alpha;
    int lineAlpha;

    StorageESP();

    void onLevelRender() override;
    void updateBlockList(BlockSource* source, const BlockPos& pos);
};
