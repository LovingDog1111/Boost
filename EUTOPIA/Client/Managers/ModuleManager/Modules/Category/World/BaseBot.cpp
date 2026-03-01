#include "BaseBot.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <vector>

#include "../../../ModuleManager.h"
#include "../Client/Client.h"
#include "../Player/PacketMine.h"
#include "../SDK/Render/MeshHelpers.h"

BaseBot::BaseBot() : Module("BaseBot", "Automatic base block scanner and TP", Category::WORLD) {
    registerSetting(new BoolSetting("Hopper", "Find Hoppers", &hopperSetting, false));
    registerSetting(new BoolSetting("Enchanting Table", "Find Enchanting Tables",
                                    &enchantingTableSetting, false));
    registerSetting(new BoolSetting("Bookshelf", "Find Bookshelves", &bookshelfSetting, false));
    registerSetting(
        new BoolSetting("Trapped Chest", "Find Trapped Chests", &trappedChestSetting, false));
    registerSetting(new BoolSetting("Ender Chest", "Find Ender Chests", &enderChestSetting, false));
    registerSetting(new BoolSetting("Anvil", "Find Anvils", &anvilSetting, false));
    registerSetting(new SliderSetting<float>(
        "TP Delay", "Delay before teleporting after finding block", &tpDelay, 1.0f, 3.0f, 10.0f));
}

void BaseBot::resettimertodefault() {
    if(dbgmsgs) {
        GI::DisplayClientMessage("[BaseBot] [DEBUG] Setting timer to 20 ticks..");
    }
    if(auto ci = GI::getClientInstance()) {
        if(auto mc = ci->minecraftSim) {
            mc->setSimTimer(20.f);
            mc->setSimSpeed(20.f / 20.f);
        }
    }
}

void BaseBot::settimertofast() {
    if(dbgmsgs) {
        GI::DisplayClientMessage("[BaseBot] [DEBUG] Setting timer to 100 ticks..");
    }
    if(auto ci = GI::getClientInstance()) {
        if(auto mc = ci->minecraftSim) {
            mc->setSimTimer(100.f);
            mc->setSimSpeed(100.f / 20.f);
        }
    }
}

bool BaseBot::isValidBlock(Block* blk) {
    if(!blk || !blk->blockLegacy)
        return false;
    int id = blk->blockLegacy->blockId;
    if(hopperSetting && id == 154)
        return true;
    if(enchantingTableSetting && id == 116)
        return true;
    if(bookshelfSetting && id == 47)
        return true;
    if(trappedChestSetting && id == 146)
        return true;
    if(enderChestSetting && id == 130)
        return true;
    if(anvilSetting && id == 145)
        return true;
    return false;
}

void BaseBot::onEnable() {
    GI::DisplayClientMessage("[BaseBot] Enabled");
    hasTarget = false;
    targetPos = Vec3<float>(0, 0, 0);
    currentActionMessage = "";
}

void BaseBot::onDisable() {
    resettimertodefault();
    GI::DisplayClientMessage("[BaseBot] Disabled");
    LocalPlayer* p = GI::getLocalPlayer();
    if(p) {
        auto* move = p->getMoveInputComponent();
        if(move) {
            move->mForward = false;
            move->mBackward = false;
            move->mLeft = false;
            move->mRight = false;
            move->mMoveVector = {0.f, 0.f};
        }
        p->setSprinting(false);
    }
    currentActionMessage = "";
}

void BaseBot::onNormalTick(LocalPlayer* player) {
    if(!player || !player->isAlive())
        return;

    BlockSource* region = GI::getClientInstance()->getRegion();
    if(!region)
        return;

    Vec3<float> pos = player->getPos();
    BlockPos base((int)pos.x, (int)pos.y, (int)pos.z);

    if(isPausedAtTarget) {
        resettimertodefault();
        BlockPos targetBlockPos(static_cast<int>(targetPos.x), static_cast<int>(targetPos.y),
                                static_cast<int>(targetPos.z));
        Block* targetBlock = region->getBlock(targetBlockPos);

        if(!isValidBlock(targetBlock)) {
            if(tpTimer == 0.f) {
                tpTimer = tpDelay;
                if(currentActionMessage != "Teleporting to next target") {
                    std::string msg = "[BaseBot] Teleporting to next target in " +
                                      std::to_string(tpDelay) + "s...";
                    GI::DisplayClientMessage(msg.c_str());
                    currentActionMessage = "Teleporting to next target";
                }
            } else {
                tpTimer -= 1.f / 20.f;
            }
            if(tpTimer <= 0.f) {
                isPausedAtTarget = false;
                tpTimer = 0.f;
                hasTarget = false;
                currentActionMessage = "";
            }
            return;
        } else {
            tpTimer = 0.f;
            return;
        }
    }

    float closestDist = FLT_MAX;
    BlockPos closestBlock(0, 0, 0);
    for(int x = -range; x <= range; x++)
        for(int y = -3; y <= 3; y++)
            for(int z = -range; z <= range; z++) {
                BlockPos bp = base.add2(x, y, z);
                if(bp.y <= 3)
                    continue;
                Block* blk = region->getBlock(bp);
                if(!isValidBlock(blk))
                    continue;

                float dist =
                    std::sqrt((pos.x - bp.x) * (pos.x - bp.x) + (pos.y - bp.y) * (pos.y - bp.y) +
                              (pos.z - bp.z) * (pos.z - bp.z));
                if(dist < closestDist) {
                    closestDist = dist;
                    closestBlock = bp;
                }
            }

    if(closestDist == FLT_MAX) {
        if(scanAttempts >= 10) {
            GI::DisplayClientMessage("[BaseBot] Failed to find target after 10 attempts.");
            Vec3<float> newPos = pos.add2(300.f, 0.f, 0.f);
            targetPos = newPos;
            hasTarget = true;
            scanAttempts = 0;
            return;
        }

        scanAttempts++;
        GI::DisplayClientMessage("[BaseBot] No blocks found, scanning again...");
        Vec3<float> newPos = pos.add2(300.f, 0.f, 0.f);
        targetPos = newPos;
        hasTarget = true;
        return;
    }

    targetPos = closestBlock.toFloat().add2(0.5f, 0.5f, 0.5f);
    hasTarget = true;

    if(closestDist < 1.0f) {
        isPausedAtTarget = true;
        return;
    }

    if(currentActionMessage != "Moving to target") {
        std::string msg = "[BaseBot] Moving to target at " + std::to_string(targetPos.x) + "," +
                          std::to_string(targetPos.y) + "," + std::to_string(targetPos.z);
        GI::DisplayClientMessage(msg.c_str());
        currentActionMessage = "Moving to target";
    }

    Vec2<float> rot = pos.CalcAngle(targetPos);
    if(auto* rotComp = player->getActorRotationComponent()) {
        rotComp->mYaw = rot.y;
        rotComp->mPitch = rot.x;
        rotComp->mOldYaw = rot.y;
        rotComp->mOldPitch = rot.x;
    }
    if(auto* headComp = player->getActorHeadRotationComponent())
        headComp->mHeadRot = rot.y;
    if(auto* bodyComp = player->getMobBodyRotationComponent())
        bodyComp->yBodyRot = rot.y;

    Vec3<float> delta = targetPos.sub(pos);
    float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    float step = std::min(1.5f, dist);
    Vec3<float> movement = delta * (step / dist);
    settimertofast();
    player->lerpMotion(movement);
    AABB a = player->getAABB(true);
    a.lower = a.lower.add(movement);
    a.upper = a.upper.add(movement);
    player->setAABB(a);
}
