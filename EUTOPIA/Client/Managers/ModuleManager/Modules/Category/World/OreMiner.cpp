#pragma once
#include "OreMiner.h"

#include "../../../ModuleManager.h"
#include "../../ModuleBase/Module.h"
#include "../Player/PacketMine.h"

bool stone = false;
static bool quartz = false;
static bool lapis = false;
bool breakPlayerBlocks = true;
bool syncwithautoeat;

OreMiner::OreMiner() : Module("OreMiner", "Mines the ores around you", Category::WORLD) {
    registerSetting(new BoolSetting("BreakPlayerBlocks", "Break blocks player is in first",
                                    &breakPlayerBlocks, true));
    // registerSetting(new BoolSetting("AutoEat", "Sync with autoeat", &syncwithautoeat, false));
    registerSetting(new SliderSetting<int>("Range", "Scan radius", &orerange, 3, 1, 12));
    registerSetting(new BoolSetting("Diamond", "Break Diamond", &diamond, true));
    registerSetting(new BoolSetting("Iron", "Break Iron", &iron, false));
    registerSetting(new BoolSetting("Gold", "Break Gold", &gold, false));
    registerSetting(new BoolSetting("Coal", "Break Coal", &coal, false));
    registerSetting(new BoolSetting("Lapis", "Break Lapis", &lapis, false));
    registerSetting(new BoolSetting("Quartz", "Break Quartz", &quartz, false));
    registerSetting(new BoolSetting("Redstone", "Break Redstone", &redstone, false));
    registerSetting(new BoolSetting("Emerald", "Break Emerald", &emerald, false));
    registerSetting(new BoolSetting("AncientDebris", "Break Ancient Debris", &ancient, false));
    targetBlock = BlockPos(0, 0, 0);
}

void OreMiner::onEnable() {
    targetBlock = BlockPos(0, 0, 0);
    itemCollected = false;
}

void OreMiner::onNormalTick(LocalPlayer* localPlayer) {
    if(!localPlayer)
        return;

    std::vector<int> ores;
    GameMode* gm = localPlayer->gamemode;
    if(!gm)
        return;

    PacketMine* packetMine = ModuleManager::getModule<PacketMine>();
    if(!packetMine || !packetMine->isEnabled())
        packetMine->setEnabled(true);

    BlockSource* region = GI::getClientInstance()->getRegion();
    if(!region)
        return;

    Baritone* baritone = ModuleManager::getModule<Baritone>();
    if(baritone && baritone->hasTarget) {
        targetBlock = baritone->targetPos.toInt();
        breakPlayerBlocks = true;
        if(baritone->mineDiamond)
            ores.push_back(56);
        if(baritone->mineIron)
            ores.push_back(15);
        if(baritone->mineGold)
            ores.push_back(14);
        if(baritone->mineCoal)
            ores.push_back(16);
        if(baritone->mineQuartz)
            ores.push_back(153);
        if(baritone->mineLapis)
            ores.push_back(21);
        if(baritone->mineRedstone)
            ores.push_back(74);
        if(baritone->mineEmerald)
            ores.push_back(129);
        if(baritone->mineAncient)
            ores.push_back(526);
    }

    Vec3<float> playerPos = localPlayer->getPos();
    BlockPos base((int)playerPos.x, (int)playerPos.y, (int)playerPos.z);

    if(diamond)
        ores.push_back(56);
    if(iron)
        ores.push_back(15);
    if(gold)
        ores.push_back(14);
    if(coal)
        ores.push_back(16);
    if(quartz)
        ores.push_back(153);
    if(lapis)
        ores.push_back(21);
    if(redstone)
        ores.push_back(74);
    if(emerald)
        ores.push_back(129);
    if(ancient)
        ores.push_back(526);
    if(stone) {
        ores.insert(ores.end(), {1, 4,  48, 98,  113, 114, 155, 491, 486, 487, 112, 3,
                                 2, 12, 13, 243, 110, 87,  88,  89,  45,  103, 18});
    }

    if(breakPlayerBlocks) {
        BlockPos playerFeetBlock((int)playerPos.x, (int)playerPos.y, (int)playerPos.z);
        BlockPos playerHeadBlock((int)playerPos.x, (int)playerPos.y + 0.5f, (int)playerPos.z);

        Block* headBlk = region->getBlock(playerHeadBlock);
        if(headBlk && headBlk->blockLegacy && headBlk->blockLegacy->blockId != 0) {
            targetBlock = playerHeadBlock;
            breakBlock(localPlayer, gm, packetMine, playerHeadBlock);
            return;
        }

        Block* feetBlk = region->getBlock(playerFeetBlock);
        if(feetBlk && feetBlk->blockLegacy) {
            int feetId = feetBlk->blockLegacy->blockId;
            if(feetId != 0) {
                targetBlock = playerFeetBlock;
                breakBlock(localPlayer, gm, packetMine, playerFeetBlock);
                return;
            }
        }
    }
    /*
     if(syncwithautoeat) {
        AutoEat* autoEat = ModuleManager::getModule<AutoEat>();
        if(autoEat && autoEat->isEnabled() && autoEat->eating) {
            return;
        }
    }*/

    BlockPos closestOre(0, 0, 0);
    float closestDist = FLT_MAX;
    bool foundBlock = false;

    BlockPos playerHeadBlock((int)playerPos.x, (int)playerPos.y + 0.5f, (int)playerPos.z);
    Block* headBlk = region->getBlock(playerHeadBlock);
    if(headBlk && headBlk->blockLegacy && headBlk->blockLegacy->blockId != 0) {
        closestOre = playerHeadBlock;
        closestDist = playerPos.dist(playerHeadBlock.CastTo<float>());
        foundBlock = true;
    }

    if(!foundBlock) {
        for(int x = -range; x <= range; x++)
            for(int y = -range; y <= range; y++)
                for(int z = -range; z <= range; z++) {
                    BlockPos pos = base.add2(x, y, z);
                    Block* blk = region->getBlock(pos);
                    if(!blk || !blk->blockLegacy)
                        continue;
                    int id = blk->blockLegacy->blockId;
                    if(std::find(ores.begin(), ores.end(), id) == ores.end())
                        continue;
                    float dist = playerPos.dist(pos.CastTo<float>());
                    if(dist < closestDist) {
                        closestDist = dist;
                        closestOre = pos;
                        foundBlock = true;
                    }
                }
    }

    if(!foundBlock) {
        itemCollected = true;
        targetBlock = BlockPos(0, 0, 0);
        return;
    }

    targetBlock = closestOre;
    breakBlock(localPlayer, gm, packetMine, closestOre);
}

void OreMiner::onUpdateRotation(LocalPlayer* localPlayer) {
    if(!localPlayer || targetBlock == BlockPos(0, 0, 0))
        return;

    Vec3<float> playerPos = localPlayer->getPos();
    Vec2<float> rot = playerPos.CalcAngle(targetBlock.toFloat().add2(0.5f, 0.5f, 0.5f));

    auto* rotComp = localPlayer->getActorRotationComponent();
    auto* headComp = localPlayer->getActorHeadRotationComponent();
    auto* bodyComp = localPlayer->getMobBodyRotationComponent();

    if(rotComp) {
        rotComp->mYaw = rot.y;
        rotComp->mPitch = rot.x;
        rotComp->mOldYaw = rot.y;
        rotComp->mOldPitch = rot.x;
    }
    if(headComp)
        headComp->mHeadRot = rot.y;
    if(bodyComp)
        bodyComp->yBodyRot = rot.y;
}

void OreMiner::breakBlock(LocalPlayer* localPlayer, GameMode* gm, PacketMine* packetMine,
                          BlockPos targetPos) {
    if(gm->destroyProgress <= 0.f || packetMine->getBreakPos() != targetPos) {
        bool b = false;
        gm->startDestroyBlock(targetPos, 0, b);
        packetMine->mineBlock(targetPos, 0);
    }
}

void OreMiner::breakBlock30(LocalPlayer* localPlayer, GameMode* gm, PacketMine* packetMine,
                            BlockPos targetPos) {
    if(gm->destroyProgress <= 0.f || packetMine->getBreakPos() != targetPos) {
        bool b = false;
        gm->startDestroyBlock(targetPos, 0, b);
        packetMine->mineBlock(targetPos, 0);
    }
}

void OreMiner::onDisable() {
    PacketMine* packetMine = ModuleManager::getModule<PacketMine>();
    if(packetMine)
        packetMine->setEnabled(false);
    targetBlock = BlockPos(0, 0, 0);
    itemCollected = false;
}