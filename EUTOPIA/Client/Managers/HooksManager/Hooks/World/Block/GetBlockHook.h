#pragma once
#include <iostream>
#include <string>

#include "../../../../ModuleManager/Modules/Category/Render/OreESP.h"
#include "../../../../NotificationManager/NotificationManager.h"
#include "../../FuncHook.h"

class GetBlockHook : public FuncHook {
   private:
    using func_t = Block*(__thiscall*)(BlockSource*, const BlockPos&);
    static inline func_t oFunc;

    static Block* BlockSourceGetBlockCallback(BlockSource* _this, const BlockPos& blockPos) {


    static WorldScanner* worldScanner = ModuleManager::getModule<WorldScanner>();
        if(worldScanner && worldScanner->isEnabled()) {
            return oFunc(_this, blockPos);  // just return original block
        }



        Block* result = oFunc(_this, blockPos);


        
   

        static PortalESP* portalEsp = ModuleManager::getModule<PortalESP>();
        static StorageESP* storageEsp = ModuleManager::getModule<StorageESP>();
        static OreESP* oreEsp = ModuleManager::getModule<OreESP>();



        if(storageEsp->isEnabled()) {
            int blockId = result->blockLegacy->blockId;
            std::string blockName = result->blockLegacy->mtranslateName;

            if(result->blockLegacy && !result->blockLegacy->mtranslateName.empty()) {
                if(storageEsp->chest && (blockId == 54 || blockId == 146)) {
                    storageEsp->blockList[blockPos] = UIColor(255, 165, 0);
                } else if(storageEsp->enderChest && blockId == 130) {
                    storageEsp->blockList[blockPos] = UIColor(255, 0, 255);
                } else if(storageEsp->shulkerBox && blockId == 205) {
                    storageEsp->blockList[blockPos] = UIColor(255, 255, 255);
                } else if(storageEsp->shulkerBox && blockId == 218) {
                    storageEsp->blockList[blockPos] = UIColor(255, 255, 255);
                } else if(storageEsp->shulkerBox && blockId == 875) {
                    storageEsp->blockList[blockPos] = UIColor(192, 192, 192);
                } else if(storageEsp->shulkerBox && blockId == 874) {
                    storageEsp->blockList[blockPos] = UIColor(128, 128, 128);
                } else if(storageEsp->shulkerBox && blockId == 882) {
                    storageEsp->blockList[blockPos] = UIColor(0, 0, 0);
                } else if(storageEsp->shulkerBox && blockId == 879) {
                    storageEsp->blockList[blockPos] = UIColor(139, 69, 19);
                } else if(storageEsp->shulkerBox && blockId == 881) {
                    storageEsp->blockList[blockPos] = UIColor(255, 0, 0);
                } else if(storageEsp->shulkerBox && blockId == 868) {
                    storageEsp->blockList[blockPos] = UIColor(255, 165, 0);
                } else if(storageEsp->shulkerBox && blockId == 871) {
                    storageEsp->blockList[blockPos] = UIColor(255, 255, 0);
                } else if(storageEsp->shulkerBox && blockId == 872) {
                    storageEsp->blockList[blockPos] = UIColor(50, 205, 50);
                } else if(storageEsp->shulkerBox && blockId == 880) {
                    storageEsp->blockList[blockPos] = UIColor(0, 100, 0);
                } else if(storageEsp->shulkerBox && blockId == 876) {
                    storageEsp->blockList[blockPos] = UIColor(0, 255, 255);
                } else if(storageEsp->shulkerBox && blockId == 870) {
                    storageEsp->blockList[blockPos] = UIColor(173, 216, 230);
                } else if(storageEsp->shulkerBox && blockId == 878) {
                    storageEsp->blockList[blockPos] = UIColor(0, 0, 255);
                } else if(storageEsp->shulkerBox && blockId == 877) {
                    storageEsp->blockList[blockPos] = UIColor(128, 0, 128);
                } else if(storageEsp->shulkerBox && blockId == 869) {
                    storageEsp->blockList[blockPos] = UIColor(255, 0, 255);
                } else if(storageEsp->shulkerBox && blockId == 873) {
                    storageEsp->blockList[blockPos] = UIColor(255, 182, 193);
                } else if(storageEsp->barrel && blockId == 458) {
                    storageEsp->blockList[blockPos] = UIColor(155, 75, 0);
                } else if(storageEsp->hopper && blockId == 154) {
                    storageEsp->blockList[blockPos] = UIColor(128, 128, 128);
                } else if(storageEsp->furnace &&
                          (blockId == 61 || blockId == 451 || blockId == 453)) {
                    storageEsp->blockList[blockPos] = UIColor(70, 70, 70);
                } else if(storageEsp->spawner && blockId == 52) {
                    storageEsp->blockList[blockPos] = UIColor(0, 0, 0);
                } else if(storageEsp->bookshelf && blockId == 47) {
                    storageEsp->blockList[blockPos] = UIColor(139, 69, 19);
                } else if(storageEsp->enchantingTable && blockId == 116) {
                    storageEsp->blockList[blockPos] = UIColor(0, 0, 255);
                } else if(storageEsp->decoratedPot && blockId == 806) {
                    storageEsp->blockList[blockPos] = UIColor(255, 0, 0);
                } else if(storageEsp->web && blockName.find("web") != std::string::npos) {
                    storageEsp->blockList[blockPos] = UIColor(128, 128, 128);
                } else if(storageEsp->dropper && blockName.find("dropper") != std::string::npos) {
                    storageEsp->blockList[blockPos] = UIColor(192, 192, 192);
                } else if(storageEsp->dispenser && blockId == 23) {
                    storageEsp->blockList[blockPos] = UIColor(169, 169, 169);
                } else if(storageEsp->anvil && blockId == 145) {
                    storageEsp->blockList[blockPos] = UIColor(105, 105, 105);
                }
            }
        }

        if(oreEsp && oreEsp->isEnabled()) {
            int blockId = result->blockLegacy->blockId;
            std::string blockName = result->blockLegacy->blockName;

            if(result->blockLegacy && !blockName.empty()) {
                if(oreEsp->diamondOre && (blockId == 56)) {
                    oreEsp->blockList[blockPos] = UIColor(0, 255, 255);
                } else if(oreEsp->goldOre && (blockId == 14)) {
                    oreEsp->blockList[blockPos] = UIColor(255, 215, 0);
                } else if(oreEsp->ironOre && (blockId == 15)) {
                    oreEsp->blockList[blockPos] = UIColor(184, 134, 11);
                } else if(oreEsp->coalOre && (blockId == 16)) {
                    oreEsp->blockList[blockPos] = UIColor(54, 54, 54);
                } else if(oreEsp->redstoneOre && (blockId == 73 || blockId == 74)) {
                    oreEsp->blockList[blockPos] = UIColor(255, 0, 0);
                } else if(oreEsp->lapisOre && (blockId == 21)) {
                    oreEsp->blockList[blockPos] = UIColor(0, 0, 255);
                } else if(oreEsp->emeraldOre && (blockId == 129)) {
                    oreEsp->blockList[blockPos] = UIColor(0, 255, 0);
                } else if(oreEsp->quartzOre && (blockId == 153)) {
                    oreEsp->blockList[blockPos] = UIColor(255, 255, 255);
                }
            }
        }

        if(portalEsp && portalEsp->isEnabled()) {
            if(result->blockLegacy && !result->blockLegacy->blockName.empty()) {
                if(result->blockLegacy->blockName.find("portal") != std::string::npos) {
                    portalEsp->blockList[blockPos] = UIColor(200, 0, 255);
                }
            }
        }

        return result;
    }

   public:
    GetBlockHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&BlockSourceGetBlockCallback;
    }
};