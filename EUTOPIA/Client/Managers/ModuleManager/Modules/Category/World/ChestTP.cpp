#include "ChestTP.h"

#include <algorithm>

#include "../../../../../Client.h"

ChestTP::ChestTP() : Module("ChestTP", "Move instantly to nearest chest", Category::WORLD) {
    registerSetting(new SliderSetting<int>("Range", "Search range", &range, 300, 10, 300));
    registerSetting(new BoolSetting("LB Mode", "Limit to Y 60-70", &lbMode, false));
    registerSetting(
        new BoolSetting("Skip Double Chests", "Avoid double chests", &skipDoubleChests, false));
}

void ChestTP::onNormalTick(LocalPlayer* player) {}


void ChestTP::onEnable() {
    Game.DisplayClientMessage("%s[ChestTP] Enabled", MCTF::GREEN);

    LocalPlayer* player = Game.getLocalPlayer();
    if(!player) {
        setEnabled(false);
        return;
    }

    BlockSource* region = Game.getClientInstance()->getRegion();
    if(!region) {
        setEnabled(false);
        return;
    }

    Vec3<float> pos = player->getPos();
    std::vector<Vec3<float>> chestPositions;

    int minX = std::max((int)(pos.x - range), 0);
    int maxX = (int)(pos.x + range);
    int minY = lbMode ? 60 : 5;
    int maxY = lbMode ? 70 : 256;
    int minZ = std::max((int)(pos.z - range), 0);
    int maxZ = (int)(pos.z + range);

    for(int x = minX; x <= maxX; ++x) {
        for(int y = minY; y <= maxY; ++y) {
            for(int z = minZ; z <= maxZ; ++z) {
                Block* block = region->getBlock(x, y, z);
                if(!block || !block->blockLegacy)
                    continue;
                if(block->blockLegacy->blockId != 54)
                    continue;

                if(skipDoubleChests) {
                    Block* right = region->getBlock(x + 1, y, z);
                    Block* front = region->getBlock(x, y, z + 1);
                    if((right && right->blockLegacy && right->blockLegacy->blockId == 54) ||
                       (front && front->blockLegacy && front->blockLegacy->blockId == 54))
                        continue;
                }

                chestPositions.push_back(Vec3<float>(x + 0.5f, y + 2.0f, z + 0.5f));
            }
        }
    }

    if(chestPositions.empty()) {
        Game.DisplayClientMessage("%s[ChestTP] No chests found", MCTF::RED);
        setEnabled(false);
        return;
    }

    Vec3<float> closest = chestPositions[0];
    float closestDist = pos.dist(closest);
    for(auto& c : chestPositions) {
        float d = pos.dist(c);
        if(d < closestDist) {
            closestDist = d;
            closest = c;
        }
    }

    Vec3<float> move = closest.sub(pos);
    player->lerpMotion(move);
    AABB a = player->getAABB(true);
    a.lower = a.lower.add(move);
    a.upper = a.upper.add(move);
    player->setAABB(a);

    Game.DisplayClientMessage("%s[ChestTP] Moved to chest", MCTF::GREEN);
    setEnabled(false);
}

void ChestTP::onDisable() {
    LocalPlayer* player = Game.getLocalPlayer();
    if(player) {
        AABB a = player->getAABB(true);
        a.upper.y = a.lower.y + 1.8f;
        player->setAABB(a);
    }
    Game.DisplayClientMessage("%s[ChestTP] Disabled", MCTF::WHITE);
}
