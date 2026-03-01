#include "OreESP.h"

#include <DrawUtil.h>

#include <chrono>

OreESP::OreESP()
    : Module("OreESP", "Highlights ore blocks", Category::RENDER),
      diamondOre(true),
      goldOre(true),
      ironOre(true),
      coalOre(true),
      redstoneOre(true),
      lapisOre(true),
      emeraldOre(true),
      quartzOre(true),
      radius(64),
      alpha(128),
      lineAlpha(255),
      updateDelayMs(200),
      lastUpdate(std::chrono::steady_clock::now()) {
    registerSetting(
        new SliderSetting<int>("Radius", "Scan radius for blocks", &radius, radius, 24, 512));
    registerSetting(new SliderSetting<int>("Alpha", "Box opacity", &alpha, alpha, 0, 255));
    registerSetting(
        new SliderSetting<int>("Line Alpha", "Outline opacity", &lineAlpha, lineAlpha, 0, 255));
    registerSetting(
        new BoolSetting("Diamond Ore", "Highlight Diamond Ore", &diamondOre, diamondOre));
    registerSetting(new BoolSetting("Gold Ore", "Highlight Gold Ore", &goldOre, goldOre));
    registerSetting(new BoolSetting("Iron Ore", "Highlight Iron Ore", &ironOre, ironOre));
    registerSetting(new BoolSetting("Coal Ore", "Highlight Coal Ore", &coalOre, coalOre));
    registerSetting(
        new BoolSetting("Redstone Ore", "Highlight Redstone Ore", &redstoneOre, redstoneOre));
    registerSetting(
        new BoolSetting("Lapis Lazuli Ore", "Highlight Lapis Lazuli Ore", &lapisOre, lapisOre));
    registerSetting(
        new BoolSetting("Emerald Ore", "Highlight Emerald Ore", &emeraldOre, emeraldOre));
    registerSetting(
        new BoolSetting("Quartz Ore", "Highlight Nether Quartz Ore", &quartzOre, quartzOre));
    registerSetting(new SliderSetting<int>("Scan Delay (ms)", "Delay between block scans",
                                           &updateDelayMs, updateDelayMs, 50, 1000));
}

void OreESP::onLevelRender() {
    auto ci = GI::getClientInstance();
    if(!ci)
        return;

    LocalPlayer* localPlayer = ci->getLocalPlayer();
    if(!localPlayer)
        return;

    AABBShapeComponent* playerAABBShape = localPlayer->getAABBShapeComponent();
    if(!playerAABBShape)
        return;

    AABB playerAABB = playerAABBShape->getAABB();
    Vec3<float> feetPos = playerAABB.getCenter();
    feetPos.y = playerAABB.lower.y;

    const BlockPos& currentPos = feetPos.add(Vec3<float>(0.f, 0.5f, 0.f)).floor().CastTo<int>();

    for(auto it = blockList.begin(); it != blockList.end();) {
        if(it->second == UIColor(0, 0, 0, 0) || abs(currentPos.x - it->first.x) > radius ||
           abs(currentPos.z - it->first.z) > radius || abs(currentPos.y - it->first.y) > radius) {
            it = blockList.erase(it);
        } else {
            ++it;
        }
    }

    auto region = ci->getRegion();
    if(!region)
        return;

    if(!blockList.empty()) {
        for(const auto& pos : blockList) {
            BlockPos blockPos = pos.first;
            Block* block = region->getBlock(blockPos);
            if(!block)
                continue;

            BlockLegacy* blockLegacy = block->blockLegcy;
            if(!blockLegacy)
                continue;

            AABB blockAABB;
            blockLegacy->getOutline(block, region, &blockPos, &blockAABB);

            UIColor boxColor = pos.second;
            DrawUtil::drawBox3dFilled(blockAABB, UIColor(boxColor.r, boxColor.g, boxColor.b, alpha),
                                      UIColor(boxColor.r, boxColor.g, boxColor.b, lineAlpha), 1.f);
        }
    }
}

void OreESP::updateBlockList(BlockSource* source, const BlockPos& pos) {
    auto now = std::chrono::steady_clock::now();
    if(std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate).count() <
       updateDelayMs)
        return;

    lastUpdate = now;

    Block* block = source->getBlock(pos);
    if(!block)
        return;

    BlockLegacy* blockLegacy = block->blockLegcy;
    if(!blockLegacy)
        return;

    int blockId = blockLegacy->blockid;
    const std::string& blockName = blockLegacy->blockName;

    if(diamondOre && (blockId == 56 || blockName.find("diamond_ore") != std::string::npos)) {
        blockList[pos] = UIColor(0, 255, 255);  // cyan
    } else if(goldOre && (blockId == 14 || blockName.find("gold_ore") != std::string::npos)) {
        blockList[pos] = UIColor(255, 215, 0);  // gold
    } else if(ironOre && (blockId == 15 || blockName.find("iron_ore") != std::string::npos)) {
        blockList[pos] = UIColor(192, 192, 192);  // silver/gray for iron
    } else if(coalOre && (blockId == 16 || blockName.find("coal_ore") != std::string::npos)) {
        blockList[pos] = UIColor(0, 0, 0);  // black
    } else if(redstoneOre &&
              (blockId == 73 || blockName.find("redstone_ore") != std::string::npos)) {
        blockList[pos] = UIColor(255, 0, 0);  // red
    } else if(lapisOre && (blockId == 21 || blockName.find("lapis_ore") != std::string::npos)) {
        blockList[pos] = UIColor(0, 0, 255);  // blue
    } else if(emeraldOre &&
              (blockId == 129 || blockName.find("emerald_ore") != std::string::npos)) {
        blockList[pos] = UIColor(0, 255, 0);  // green
    } else if(quartzOre &&
              (blockId == 153 || blockName.find("nether_quartz_ore") != std::string::npos)) {
        blockList[pos] = UIColor(255, 255, 255);  // white
    } else {
        blockList.erase(pos);
    }
}
