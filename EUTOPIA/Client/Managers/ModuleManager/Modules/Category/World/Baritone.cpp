#define NOMINMAX
#include "Baritone.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <unordered_set>
#include <vector>

#include "../../../ModuleManager.h"
#include "../../SDK/GlobalInstance.h"
#include "../../SDK/NetWork/Packets/CommandRequestPacket.h"
#include "../Client/Client.h"
#include "../Player/PacketMine.h"
#include "../SDK/Render/MeshHelpers.h"
#include "..\\Client\\Managers\\ModuleManager\\Modules\\Category\\World\\OreMiner.h"

BlockPos targetBlockPos;
bool isPausedAtTarget = false;
float tpDelay = 3.f;
float tpTimer = 0.f;
static bool stone = false;
std::string currentActionMessage = "";
bool chests = false;
int minCluster = 1;
std::vector<BlockPos> currentVein;
float normaltimer = 20.f;
float spedup = 100.f;
bool dbgmsgs = false;
bool timersettingz;
int minChestCluster = 1;
bool classicMode = false;
bool StashBlocks;
bool canAutoBreak;
bool antilava;
bool AutoLog2;
int healthThreshold = 20;
bool antiBedrock;
bool smborderanti;
bool eatauto;

static bool isValidBlockPtr(Block* b) {
    return b && b->blockLegacy;
}

bool isValidOre(Block* blk, const std::vector<int>& ores) {
    if(!isValidBlockPtr(blk))
        return false;
    int id = blk->blockLegacy->blockId;
    return std::find(ores.begin(), ores.end(), id) != ores.end();
}

void Baritone::onNormalTick(LocalPlayer* player) {
    if(!player || !player->isAlive())
        return;

    auto* ci = GI::getClientInstance();
    if(!ci)
        return;

    BlockSource* region = ci->getRegion();
    if(!region)
        return;

    Vec3<float> pos = player->getPos();

    if(AutoLog2 && player->getHealth() > 0.f && player->getHealth() <= healthThreshold) {
        this->setEnabled(false);
        return;
    }

    BlockPos base((int)pos.x, (int)pos.y, (int)pos.z);

    std::vector<int> ores;
    if(mineDiamond)
        ores.push_back(56);
    if(mineIron)
        ores.push_back(15);
    if(mineGold)
        ores.push_back(14);
    if(mineCoal)
        ores.push_back(16);
    if(mineQuartz)
        ores.push_back(153);
    if(mineLapis)
        ores.push_back(21);
    if(mineRedstone)
        ores.push_back(74);
    if(mineEmerald)
        ores.push_back(129);
    if(mineAncient)
        ores.push_back(526);
    if(chests)
        ores.push_back(54);

    float closestDist = FLT_MAX;
    BlockPos closestOre(0, 0, 0);

    for(int x = -range; x <= range; x++) {
        for(int y = -3; y <= 3; y++) {
            for(int z = -range; z <= range; z++) {
                BlockPos bp = base.add2(x, y, z);
                if(bp.y <= 3)
                    continue;

                Block* blk = region->getBlock(bp);
                if(!isValidOre(blk, ores))
                    continue;

                float dist =
                    std::sqrt((pos.x - bp.x) * (pos.x - bp.x) + (pos.y - bp.y) * (pos.y - bp.y) +
                              (pos.z - bp.z) * (pos.z - bp.z));

                if(dist < closestDist) {
                    closestDist = dist;
                    closestOre = bp;
                }
            }
        }
    }

    if(closestDist == FLT_MAX) {
        hasTarget = false;
        return;
    }

    Block* targetBlk = region->getBlock(closestOre);
    if(!isValidBlockPtr(targetBlk))
        return;

    targetBlockPos = closestOre;
    targetPos = closestOre.toFloat().add2(0.5f, 0.5f, 0.5f);
    hasTarget = true;

    Vec3<float> delta = targetPos.sub(pos);
    float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    if(dist <= 0.001f || std::isnan(dist) || std::isinf(dist))
        return;

    float step = std::min(1.5f, dist);
    float scale = step / dist;

    Vec3<float> movement(delta.x * scale, delta.y * scale, delta.z * scale);

    player->lerpMotion(movement);

    AABB a = player->getAABB(true);
    a.lower = a.lower.add(movement);
    a.upper = a.upper.add(movement);
    player->setAABB(a);
}