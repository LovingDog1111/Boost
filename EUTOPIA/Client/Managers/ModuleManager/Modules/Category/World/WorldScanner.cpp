#include "WorldScanner.h"

#include <algorithm>
#include <cmath>


float scanDelay = 10.f;
float scanDelayTimer = 0.f;
bool waitingForNext = false;
bool justArrived = false;
int startY;
int scanY;


WorldScanner::WorldScanner()
    : Module("WorldScanner", "Scans the entire world for stash blocks over time", Category::WORLD) {
    registerSetting(new BoolSetting("Hopper", "Scan Hoppers", &hopperSetting, false));
    registerSetting(new BoolSetting("Enchanting Table", "Scan Enchanting Tables",
                                    &enchantingTableSetting, false));
    registerSetting(new BoolSetting("Chest", "Scan Chests", &chestSetting, false));
    registerSetting(new BoolSetting("Bookshelf", "Scan Bookshelves", &bookshelfSetting, false));
    registerSetting(
        new BoolSetting("Trapped Chest", "Scan Trapped Chests", &trappedChestSetting, false));
    registerSetting(new BoolSetting("Ender Chest", "Scan Ender Chests", &enderChestSetting, false));
    registerSetting(new BoolSetting("Anvil", "Scan Anvils", &anvilSetting, false));
    registerSetting(new BoolSetting("Furnace", "Scan Furnaces", &furnaceSetting, false));
    registerSetting(new SliderSetting<float>("Scan Delay", "Delay between scan teleports",
                                             &scanDelay, 10.f, 1.f, 30.f));

    stashBlocks = {{145, "Hopper"},   {116, "Enchanting Table"}, {54, "Chest"},
                   {47, "Bookshelf"}, {146, "Trapped Chest"},    {130, "Ender Chest"},
                   {149, "Anvil"},    {117, "Furnace"}};
}

void WorldScanner::onEnable() {
    LocalPlayer* lp = Game.getLocalPlayer();
    if(!lp)
        return;

    scanX = minX;
    scanZ = minZ;
    scanY = minY;

    Utils::resetTimer();
    startPos = lp->getPos();

    scanPos = Vec3<float>((float)scanX, (float)scanY, (float)scanZ);

    totalSteps = ((maxX - minX) / step + 1) * ((maxZ - minZ) / step + 1) * ((maxY - minY) + 1);
    completedSteps = 0;

    foundBlocks.clear();
    tpCooldown = 0.f;
    waitingForNext = false;
    scanDelayTimer = 0.f;
    justArrived = false;
    currentAction.clear();

    Game.DisplayClientMessage("[WorldScanner] Started world scan", MCTF::WHITE);
}


void WorldScanner::onDisable() {
    Utils::resetTimer();
    waitingForNext = false;
    scanDelayTimer = 0.f;
}

void WorldScanner::scanArea(BlockSource* region, const Vec3<float>& center) {
    int cx = (int)center.x;
    int cz = (int)center.z;
    int r = step / 2;

    for(int x = std::max(cx - r, minX); x <= std::min(cx + r, maxX); x++) {
        for(int y = minY; y <= maxY; y++) {
            for(int z = std::max(cz - r, minZ); z <= std::min(cz + r, maxZ); z++) {
                BlockPos bp(x, y, z);
                Block* b = region->getBlock(bp);
                if(!b || !b->blockLegacy)
                    continue;

                int id = b->blockLegacy->blockId;
                auto it = stashBlocks.find(id);
                if(it == stashBlocks.end())
                    continue;

                if((id == 145 && !hopperSetting) || (id == 116 && !enchantingTableSetting) ||
                   (id == 54 && !chestSetting) || (id == 47 && !bookshelfSetting) ||
                   (id == 146 && !trappedChestSetting) || (id == 130 && !enderChestSetting) ||
                   (id == 149 && !anvilSetting) || (id == 117 && !furnaceSetting))
                    continue;

                foundBlocks.push_back(bp);
            }
        }
    }
}

void WorldScanner::onNormalTick(LocalPlayer* player) {
    if(!player)
        return;

    BlockSource* region = GI::getClientInstance()->getRegion();
    if(!region)
        return;

    if(waitingForNext) {
        scanDelayTimer -= 1.f / 20.f;
        if(scanDelayTimer > 0.f) {
            std::string msg =
                "[WorldScanner] Waiting " + std::to_string((int)std::ceil(scanDelayTimer)) + "s";
            if(currentAction != msg) {
                Game.DisplayClientMessage(msg.c_str(), MCTF::WHITE);
                currentAction = msg;
            }
            return;
        }
        waitingForNext = false;
    }


    if(tpCooldown > 0.f) {
        tpCooldown -= 1.f / 20.f;
        return;
    }

    Vec3<float> pos = player->getPos();
    Vec3<float> delta = scanPos.sub(pos);
    float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);


    if(dist > 0.5f) {
        Vec3<float> move = delta * (1.5f / dist);
        player->lerpMotion(move);

        AABB box = player->getAABB(true);
        box.lower = box.lower.add(move);
        box.upper = box.upper.add(move);
        player->setAABB(box);
        return;
    }

    scanArea(region, scanPos);

    waitingForNext = true;
    scanDelayTimer = scanDelay;

    completedSteps++;
    float percent = std::min(100.f, (completedSteps / (float)totalSteps) * 100.f);
    std::string msg = "[WorldScanner] Scanning " + std::to_string((int)percent) + "%";
    if(currentAction != msg) {
        Game.DisplayClientMessage(msg.c_str(), MCTF::WHITE);
        currentAction = msg;
    }

    scanX += step;
    if(scanX > maxX) {
        scanX = minX;
        scanZ += step;
        if(scanZ > maxZ) {
            scanZ = minZ;
            startY += 1; 
            if(startY > maxY) {
                Game.DisplayClientMessage("[WorldScanner] Scan complete (100%)", MCTF::GREEN);
                for(auto& bp : foundBlocks) {
                    char buf[256];
                    snprintf(buf, sizeof(buf),
                             "[WorldScanner] Found stash at X%s%d%s Y%s%d%s Z%s%d%s", MCTF::AQUA,
                             bp.x, MCTF::WHITE, MCTF::GREEN, bp.y, MCTF::WHITE, MCTF::RED, bp.z,
                             MCTF::WHITE);
                    Game.DisplayClientMessage(buf, MCTF::WHITE);
                }
                setEnabled(false);
                return;
            }
        }
    }

    scanPos = Vec3<float>((float)scanX, (float)startY, (float)scanZ);

    tpCooldown = 0.2f;
}
