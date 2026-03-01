#pragma once
#include <algorithm>
#include <unordered_map>
#undef max
#include "../../ModuleBase/Module.h"

class StashFinder : public Module {
   private:
    int range = 300;

    bool hopperSetting = false;
    bool enchantingTableSetting = false;
    bool chestSetting = false;
    bool bookshelfSetting = false;
    bool trappedChestSetting = false;
    bool enderChestSetting = false;
    bool anvilSetting = false;
    bool furnaceSetting = false;

    std::unordered_map<int, std::string> stashBlocks = {
        {145, "Hopper"},        {116, "Enchanting Table"}, {54, "Chest"},  {47, "Bookshelf"},
        {146, "Trapped Chest"}, {130, "Ender Chest"},      {149, "Anvil"}, {117, "Furnace"}};

   public:
    StashFinder()
        : Module("StashFinder", "Find stash blocks and show their coordinates", Category::WORLD) {
        registerSetting(
            new SliderSetting<int>("Range", "Search range for stash blocks", &range, 300, 10, 300));
        registerSetting(new BoolSetting("Hopper", "Find Hoppers", &hopperSetting, false));
        registerSetting(new BoolSetting("Enchanting Table", "Find Enchanting Tables",
                                        &enchantingTableSetting, false));
        registerSetting(new BoolSetting("Chest", "Find Chests", &chestSetting, false));
        registerSetting(new BoolSetting("Bookshelf", "Find Bookshelves", &bookshelfSetting, false));
        registerSetting(
            new BoolSetting("Trapped Chest", "Find Trapped Chests", &trappedChestSetting, false));
        registerSetting(
            new BoolSetting("Ender Chest", "Find Ender Chests", &enderChestSetting, false));
        registerSetting(new BoolSetting("Anvil", "Find Anvils", &anvilSetting, false));
        registerSetting(new BoolSetting("Furnace", "Find Furnaces", &furnaceSetting, false));
    }

    void onEnable() override {
        LocalPlayer* localPlayer = Game.getLocalPlayer();
        if(!localPlayer)
            return;

        BlockSource* region = GI::getClientInstance()->getRegion();
        if(!region)
            return;

        Vec3<float> playerPos = localPlayer->getPos();

        int minX = std::max((int)playerPos.x - range, 0);
        int maxX = (int)playerPos.x + range;
        int minY = 5;
        int maxY = 128;
        int minZ = std::max((int)playerPos.z - range, 0);
        int maxZ = (int)playerPos.z + range;

        bool foundAny = false;

        for(int x = minX; x <= maxX; ++x) {
            for(int y = minY; y <= maxY; ++y) {
                for(int z = minZ; z <= maxZ; ++z) {
                    BlockPos blockPos(x, y, z);
                    Block* block = region->getBlock(blockPos);
                    if(!block || !block->blockLegacy)
                        continue;

                    int blockId = block->blockLegacy->blockId;
                    auto it = stashBlocks.find(blockId);
                    if(it == stashBlocks.end())
                        continue;

                    if((blockId == 145 && !hopperSetting) ||
                       (blockId == 116 && !enchantingTableSetting) ||
                       (blockId == 54 && !chestSetting) || (blockId == 47 && !bookshelfSetting) ||
                       (blockId == 146 && !trappedChestSetting) ||
                       (blockId == 130 && !enderChestSetting) ||
                       (blockId == 149 && !anvilSetting) || (blockId == 117 && !furnaceSetting))
                        continue;

                    foundAny = true;
                    char buf[256];
                    snprintf(buf, sizeof(buf), "[StashFinder] Found %s at X%s%d%s Y%s%d%s Z%s%d%s",
                             it->second.c_str(), MCTF::AQUA, x, MCTF::WHITE, MCTF::GREEN, y,
                             MCTF::WHITE, MCTF::RED, z, MCTF::WHITE);
                    Game.DisplayClientMessage(buf, MCTF::WHITE);
                }
            }
        }

        if(!foundAny)
            Game.DisplayClientMessage("[StashFinder] No stash blocks found in range.", MCTF::RED);

        this->setEnabled(false);
    }
};
