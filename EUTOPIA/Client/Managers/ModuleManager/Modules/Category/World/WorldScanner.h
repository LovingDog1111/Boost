#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "../../ModuleBase/Module.h"
#include "../../SDK/GlobalInstance.h"

class WorldScanner : public Module {
   private:
    int step = 300;
    int minX = 1;
    int maxX = 1199;
    int minZ = 1;
    int maxZ = 1199;
    int minY = 5;
    int maxY = 128;

    bool hopperSetting = false;
    bool enchantingTableSetting = false;
    bool chestSetting = false;
    bool bookshelfSetting = false;
    bool trappedChestSetting = false;
    bool enderChestSetting = false;
    bool anvilSetting = false;
    bool furnaceSetting = false;

    std::unordered_map<int, std::string> stashBlocks;
    std::vector<BlockPos> foundBlocks;

    Vec3<float> startPos;
    Vec3<float> scanPos;
    int scanX;
    int scanZ;
    int totalSteps;
    int completedSteps;
    float tpCooldown;
    std::string currentAction;

    void scanArea(BlockSource* region, const Vec3<float>& center);

   public:
    WorldScanner();
    void onEnable() override;
    void onDisable() override;
    void onNormalTick(LocalPlayer* player) override;
};
