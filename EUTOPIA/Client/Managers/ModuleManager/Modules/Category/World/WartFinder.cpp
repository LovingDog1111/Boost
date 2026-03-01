#include "WartFinder.h"

WartFinder::WartFinder() : Module("WartFinder", "Find Nether Warts instantly", Category::WORLD) {
    registerSetting(new SliderSetting<int>("Range", "Max search range", &range, 300, 10, 300));
    registerSetting(new BoolSetting("Nether Wart", "Find Nether Warts", &netherWartSetting, true));
}

void WartFinder::onNormalTick(LocalPlayer* player) {}

void WartFinder::onEnable() {
    LocalPlayer* localPlayer = Game.getLocalPlayer();
    if(!localPlayer)
        return;

    BlockSource* region = GI::getClientInstance()->getRegion();
    if(!region)
        return;

    Vec3<float> playerPos = localPlayer->getPos();

    int minX = std::max((int)playerPos.x - range, 0);
    int maxX = (int)playerPos.x + range;
    int minY = 1;
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
                if(blockId != 115 || !netherWartSetting)
                    continue;

                foundAny = true;
                char buf[256];
                snprintf(buf, sizeof(buf),
                         "[WartFinder] Found Nether Wart at X%s%d%s Y%s%d%s Z%s%d%s", MCTF::AQUA, x,
                         MCTF::WHITE, MCTF::GREEN, y, MCTF::WHITE, MCTF::RED, z, MCTF::WHITE);
                Game.DisplayClientMessage(buf, MCTF::WHITE);
            }
        }
    }

    if(!foundAny)
        Game.DisplayClientMessage("[WartFinder] No Nether Warts found in range", MCTF::RED);

    this->setEnabled(false);
}
