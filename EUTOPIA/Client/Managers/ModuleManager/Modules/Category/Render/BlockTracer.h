#pragma once
#include <string>
#include <unordered_map>

#include "../../../ModuleManager.h"
#include "../../ModuleBase/Module.h"

class BlockTracer : public Module {
   public:
    BlockTracer();

    void onEnable() override;
    void onLevelRender() override;


    void setTargetBlock(int targetBlockId);

   private:
    std::string targetBlockName;
    int targetBlockId = -1;  
    Vec3<float> targetPos;  
    bool hasTarget = false;
    int range = 50;

    void drawTracerToBlock(const Vec3<float>& blockPos);


    std::unordered_map<std::string, int> blockNameToId = {
        {"Hopper", 145},        {"Enchanting Table", 116}, {"Chest", 54},  {"Bookshelf", 47},
        {"Trapped Chest", 146}, {"Ender Chest", 130},      {"Anvil", 149}, {"Furnace", 117}};
};
