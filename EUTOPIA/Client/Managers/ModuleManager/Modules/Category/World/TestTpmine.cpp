#include "TestTpmine.h"
// made by daddy smking //

#include <Minecraft/PlayerUtil.h>
#include <Windows.h>

#include <cfloat>
#include <cmath>
#include <queue>
#include <unordered_set>

void TestTpmine::IntList::push(int v) {
    if(n < 64)
        data[n++] = v;
}

bool TestTpmine::IntList::contains(int v) const {
    for(int i = 0; i < n; ++i)
        if(data[i] == v)
            return true;
    return false;
}

TestTpmine::TestTpmine() : Module("TpMine", "Teleport to ores", Category::WORLD) {
    registerSetting(new SliderSetting<int>("Range", "Search range for ores", &range, 30, 3, 50));
    registerSetting(new SliderSetting<int>("Vein Size", "Min cluster size", &clusterSize, 1, 1, 9));
    registerSetting(new BoolSetting("AntiGravel", "Avoid ores under gravel", &antiGravel, false));
    registerSetting(new BoolSetting("AntiLava", "Avoid ores near lava", &antiLava, true));
    registerSetting(new BoolSetting("Diamond", "Target Diamond ore", &dim, false));
    registerSetting(new BoolSetting("Iron", "Target Iron ore", &iron, false));
    registerSetting(new BoolSetting("Coal", "Target Coal ore", &coal, false));
    registerSetting(new BoolSetting("Gold", "Target Gold ore", &gold, false));
    registerSetting(new BoolSetting("Emerald", "Target Emerald ore", &emerald, false));
    registerSetting(new BoolSetting("Redstone", "Target Redstone ore", &redstone, false));
    registerSetting(new BoolSetting("Quartz", "Target Quartz ore", &quartz, false));
    registerSetting(new BoolSetting("Lapis", "Target Lapis ore", &lapis, false));
    registerSetting(new BoolSetting("Spawners", "Target Spawners", &spawners, false));
    registerSetting(new BoolSetting("Custom", "Enable custom block ID", &cuOre, false));
    registerSetting(new SliderSetting<int>("CustomID", "Custom Block ID", &cuID, 1, 0, 252));
}

bool TestTpmine::checkClusterValidity(BlockSource* region, int x, int y, int z, int blockId) {
    if(antiGravel) {
        Block* aboveBlock = region->getBlock(BlockPos(x, y + 1, z));
        if(aboveBlock && aboveBlock->blockLegacy && aboveBlock->blockLegacy->blockId == 13) {
            return false;
        }
    }
    if(antiBorder) {
        if(x > 1090 || x < 50 || z > 1090 || z < 50) {
            return false;
        }
    }
    if(antiLava) {
        for(int lx = -10; lx <= 10; lx++) {
            for(int ly = -10; ly <= 10; ly++) {
                for(int lz = -10; lz <= 10; lz++) {
                    BlockPos lp = BlockPos(x + lx, y + ly, z + lz);
                    Block* lb = region->getBlock(lp);
                    if(lb && lb->blockLegacy) {
                        int lavaId = lb->blockLegacy->blockId;
                        if(lavaId == 10 || lavaId == 11) {
                            return false;
                        }
                    }
                }
            }
        }
    }
    return true;
}

int TestTpmine::findConnectedVeinSize(BlockSource* region, int startX, int startY, int startZ,
                                      int blockId) {
    std::unordered_set<uint64_t> visited;
    std::queue<BlockPos> queue;
    int veinSize = 0;

    auto getHash = [](const BlockPos& pos) {
        return ((uint64_t)pos.x << 32) | ((uint64_t)pos.y << 16) | (uint64_t)pos.z;
    };

    queue.push(BlockPos(startX, startY, startZ));
    visited.insert(getHash(BlockPos(startX, startY, startZ)));

    while(!queue.empty() && veinSize < clusterSize) {
        BlockPos current = queue.front();
        queue.pop();
        veinSize++;

        const int offsets[6][3] = {{1, 0, 0},  {-1, 0, 0}, {0, 1, 0},
                                   {0, -1, 0}, {0, 0, 1},  {0, 0, -1}};

        for(int i = 0; i < 6; i++) {
            BlockPos neighbor(current.x + offsets[i][0], current.y + offsets[i][1],
                              current.z + offsets[i][2]);

            uint64_t hash = getHash(neighbor);
            if(visited.find(hash) != visited.end()) {
                continue;
            }

            Block* block = region->getBlock(neighbor);
            if(!block || !block->blockLegacy) {
                continue;
            }

            if(block->blockLegacy->blockId == blockId) {
                if(checkClusterValidity(region, neighbor.x, neighbor.y, neighbor.z, blockId)) {
                    queue.push(neighbor);
                    visited.insert(hash);
                }
            }
        }
    }

    return veinSize;
}

void TestTpmine::findOres() {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;
    BlockSource* region = GI::getClientInstance()->getRegion();
    if(!region)
        return;
    Vec3<float> playerPos = localPlayer->getPos();
    int minX = std::max((int)playerPos.x - range, 0);
    int maxX = (int)playerPos.x + range;
    int minZ = std::max((int)playerPos.z - range, 0);
    int maxZ = (int)playerPos.z + range;
    IntList targetOres;
    if(coal)
        targetOres.push(16);
    if(iron)
        targetOres.push(15);
    if(gold)
        targetOres.push(14);
    if(redstone)
        targetOres.push(74);
    if(dim)
        targetOres.push(56);
    if(emerald)
        targetOres.push(129);
    if(ancient)
        targetOres.push(526);
    if(quartz)
        targetOres.push(153);
    if(lapis)
        targetOres.push(21);
    if(spawners)
        targetOres.push(52);
    if(cuOre)
        targetOres.push(cuID);

    std::unordered_set<uint64_t> processedVeins;
    auto getHash = [](int x, int y, int z) {
        return ((uint64_t)x << 32) | ((uint64_t)y << 16) | (uint64_t)z;
    };

    for(int x = minX; x <= maxX; x++) {
        for(int y = 5; y <= 128; y++) {
            for(int z = minZ; z <= maxZ; z++) {
                BlockPos blockPos(x, y, z);
                Block* block = region->getBlock(blockPos);
                if(!block || !block->blockLegacy)
                    continue;
                int blockId = block->blockLegacy->blockId;
                if(!targetOres.contains(blockId))
                    continue;

                uint64_t hash = getHash(x, y, z);
                if(processedVeins.find(hash) != processedVeins.end())
                    continue;

                if(!checkClusterValidity(region, x, y, z, blockId))
                    continue;

                int veinSize = findConnectedVeinSize(region, x, y, z, blockId);

                if(veinSize < clusterSize)
                    continue;

                processedVeins.insert(hash);

                float dist = playerPos.dist(Vec3<float>(x, y, z));
                if(dist < closestDistance) {
                    closestDistance = dist;
                    goal = Vec3<float>(x + 0.5f, y + 0.5f, z + 0.5f);
                    oreBlockPos = Vec3<int>(x, y, z);
                }
            }
        }
    }
}

void TestTpmine::centerPlayerOnBlock(LocalPlayer* player) {
    if(!player)
        return;

    Vec3<float> targetPos(oreBlockPos.x + 0.5f, oreBlockPos.y + 1.0f, oreBlockPos.z + 0.5f);

    Vec3<float> currentPos = player->getPos();
    Vec3<float> delta = targetPos - currentPos;

    player->lerpMotion(delta);

    AABB currentAABB = player->getAABB(true);
    currentAABB.lower = currentAABB.lower + delta;
    currentAABB.upper = currentAABB.upper + delta;
    player->setAABB(currentAABB);

    GI::DisplayClientMessage("%s[%sTpMine%s] %sPlayer centered on ore", MCTF::GRAY,
                             MCTF::BLUE, MCTF::GRAY, MCTF::WHITE);
}

void TestTpmine::onEnable() {
    GI::DisplayClientMessage("%s[%sTpMine%s] %sEnabled", MCTF::GRAY, MCTF::GREEN, MCTF::GRAY,
                             MCTF::WHITE);
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;
    BlockSource* region = GI::getClientInstance() ? GI::getClientInstance()->getRegion() : nullptr;
    if(!region)
        return;
    if(auto ci = GI::getClientInstance()) {
        if(auto mc = ci->minecraftSim) {
            prevTimer = mc->getSimTimer();
            mc->setSimTimer(static_cast<float>(timerValue));
            float speed = static_cast<float>(timerValue) / 20.0f;
            mc->setSimSpeed(speed);
        }
    }
    AABB currentAABB = localPlayer->getAABB(true);
    currentAABB.upper.y = currentAABB.lower.y;
    localPlayer->setAABB(currentAABB);
    closestDistance = FLT_MAX;
    goal = Vec3<float>(0, 0, 0);
    oreBlockPos = Vec3<int>(0, 0, 0);
    works = false;
    findOres();
    if(closestDistance < FLT_MAX) {
        works = true;
        GI::DisplayClientMessage("%s[%sTpMine%s] %sFound ore cluster at X: %d Y: %d Z: %d",
                                 MCTF::GRAY, MCTF::BLUE, MCTF::GRAY, MCTF::WHITE,
                                 oreBlockPos.x, oreBlockPos.y, oreBlockPos.z);
    } else {
        GI::DisplayClientMessage("%s[%sTpMine%s] %sNo ores found", MCTF::GRAY, MCTF::BLUE,
                                 MCTF::GRAY, MCTF::RED);
        this->setEnabled(false);
    }
}

void TestTpmine::onNormalTick(LocalPlayer* player) {
    if(!player || !works)
        return;
    Vec3<float> cur = player->getPos();
    float dX = goal.x - cur.x;
    float dY = goal.y - cur.y;
    float dZ = goal.z - cur.z;
    float dist = std::sqrt(dX * dX + dY * dY + dZ * dZ);
    if(dist <= 1.2f) {
        if(centerPlayer) {
            centerPlayerOnBlock(player);
        }

        GI::DisplayClientMessage("%s[%sTpMine%s] %sReached ore at X: %d Y: %d Z: %d", MCTF::GRAY,
                                 MCTF::BLUE, MCTF::GRAY, MCTF::WHITE, oreBlockPos.x,
                                 oreBlockPos.y, oreBlockPos.z);
        this->setEnabled(false);
        return;
    }
    float currentSpeed = tpSpeed;
    if(midClickTP && (GetAsyncKeyState(VK_MBUTTON) & 0x8000)) {
        currentSpeed = tpSpeed * 2.0f;
    }
    if(dist > 0.001f) {
        float step = currentSpeed;
        if(step > dist)
            step = dist;
        float s = step / dist;
        Vec3<float> delta(dX * s, dY * s, dZ * s);
        player->lerpMotion(delta);
        AABB currentAABB = player->getAABB(true);
        currentAABB.lower = currentAABB.lower.add(delta);
        currentAABB.upper = currentAABB.upper.add(delta);
        player->setAABB(currentAABB);
    }
}

void TestTpmine::onDisable() {
    if(auto ci = GI::getClientInstance()) {
        if(auto mc = ci->minecraftSim) {
            mc->setSimTimer(prevTimer);
            mc->setSimSpeed(prevTimer / 20.0f);
        }
    }
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(localPlayer) {
        AABB currentAABB = localPlayer->getAABB(true);
        currentAABB.upper.y = currentAABB.lower.y + 1.8f;
        localPlayer->setAABB(currentAABB);
    }
    GI::DisplayClientMessage("%s[%TpMine%s] %sDisabled", MCTF::GRAY, MCTF::BLUE,
                             MCTF::GRAY, MCTF::WHITE);
}
// made by daddy smking //