#include "PacketMine.h"

#include "../Utils\Minecraft\InvUtil.h"
#include "../../../ModuleManager.h"
#include "../Utils\Minecraft\WorldUtil.h"
#include "..\SDK\NetWork\Packets\MobEquipmentPacket.h"

PacketMine::PacketMine() : Module("PacketMine", "Mines using packets", Category::PLAYER) {
    registerSetting(new SliderSetting<float>("Speed", "Mining speed", &mineSpeed, 1.f, 0.1f, 2.f));
    registerSetting(
        new SliderSetting<float>("Mine Range", "Mining range", &mineRange, 6.f, 1.f, 12.f));
    registerSetting(new SliderSetting<float>(
        "Continue Reset", "The break progress at the 2nd and more mine if using continue mode",
        &continueReset, 1.f, 0.f, 1.f));
    registerSetting(new EnumSetting("Mine Type",
                                    "Mining Type\nNormal: Mines normally\nContinue: Mines the same "
                                    "block over and over again\nInstant: Instamines all blocks",
                                    {"Normal", "Continue", "Instant"}, &mineType, 0));
    registerSetting(new EnumSetting(
        "MultiTask",
        "Multitasking mode\nNormal: When eating the client will continue mining but will only "
        "break when the player is no longer eating/doing something\nStrict: Breaking will reset "
        "when eating/bowing/using an item\nStrict2: Eating will reset when placing, breaking, "
        "attacking, using an item\nNone: Mines even if doing something",
        {"Normal", "Strict", "Strict2", "None"}, &multiTask, 0));
    registerSetting(new BoolSetting(
        "Switch Back",
        "Switches back to old slot after mining, this may nullify the mine on some servers",
        &switchBack, false));
}

bool PacketMine::canBreakBlock(const BlockPos& blockPos) {
    if(blockPos == BlockPos(0, 0, 0))
        return false;
    if(blockPos.CastTo<float>().dist(Game.getLocalPlayer()->getPos()) > mineRange)
        return false;
    if(WorldUtil::getBlock(blockPos)->blockLegacy->canBeBuiltOver(
           Game.getClientInstance()->getRegion(),
                                                                  blockPos))
        return false;
    if(WorldUtil::getBlock(blockPos)->blockLegacy->blockId == 7)
        return false;
    return true;
}

bool PacketMine::sendBreakPacket(const BlockPos& blockPos, uint8_t face, LocalPlayer* localPlayer) {
    return true;
}
bool SpoofSwitchezels(int slot) {
    auto player = Game.getLocalPlayer();
    if(!player)
        return false;
    auto itemStack = player->getsupplies()->container->getItem(slot);
    if(!itemStack) {
        return false;
    }

    auto mep = MinecraftPacket::createPacket(PacketID::MobEquipment);
    auto* pkt = reinterpret_cast<MobEquipmentPacket*>(mep.get());
    // auto* pkt = reinterpret_cast<MobEquipmentPacket*>(mep.get());

    // pkt->mRuntimeId = player->getComponent<RuntimeIDComponent>()->runtimeID;
    pkt->mSlot = slot;
    pkt->mSelectedSlot = slot;
    pkt->mContainerId = 0;
    pkt->mSlotByte = slot;
    pkt->mSelectedSlotByte = slot;
    pkt->mContainerIdByte = 0;

    Game.getPacketSender()->sendToServer(pkt);

    return true;
}

void PacketMine::reset() {
    LocalPlayer* localPlayer = Game.getLocalPlayer();
    breakPair.first = BlockPos(0, 0, 0);
    breakPair.second = -1;
    localPlayer->gamemode->destroyProgress = 0;
}

bool PacketMine::breakBlock(const BlockPos& blockPos, uint8_t face) {
    LocalPlayer* localPlayer = Game.getLocalPlayer();
    GameMode* gm = localPlayer->gamemode;
    gm->destroyBlock(blockPos, face);
    // sendBreakPacket(blockPos, face, localPlayer); apparently nukkit dont support this type of
    // packet for handling break but geyser does
    if(mineType != 1) {
        breakPair = std::make_pair(BlockPos(0, 0, 0), -1);
        Game.getLocalPlayer()->gamemode->lastBreakPos = BlockPos(0, 0, 0);
    } else
        Game.getLocalPlayer()->gamemode->destroyProgress = continueReset;
    return true;
}

void PacketMine::mineBlock(const BlockPos& blockPos, uint8_t face) {
    if(!canBreakBlock(blockPos))
        return;
    if(mineType == 2)
        Game.getLocalPlayer()->gamemode->destroyProgress = 1.f;
    else
        Game.getLocalPlayer()->gamemode->destroyProgress = 0.f;
    breakPair = std::make_pair(blockPos, face);
}

std::pair<float, uint8_t> PacketMine::getMineTool(const BlockPos& blockPos) {
    LocalPlayer* localPlayer = Game.getLocalPlayer();
    GameMode* gm = localPlayer->gamemode;
    Block* block = WorldUtil::getBlock(blockPos);
    uint8_t oldSlot = InvUtil::getSelectedSlot();
    uint8_t bestSlot = 0;
    if(block != nullptr) {
        float destroyRate = gm->getDestroyRate(block);
        for(uint8_t i = 0; i < 9; i++) {
            InvUtil::switchSlot(i);
            const float value = gm->getDestroyRate(block);
            if(value > destroyRate) {
                destroyRate = value;
                bestSlot = i;
            }
        }
        InvUtil::switchSlot(oldSlot);
        return std::make_pair(destroyRate, bestSlot);
    }
    return std::make_pair(0.f, 0);
}

void PacketMine::onNormalTick(LocalPlayer* localPlayer) {
    static bool shouldSwitch = false;
    static uint8_t lastSlot = -1;
    GameMode* gm = localPlayer->gamemode;
    if(gm == nullptr)
        return;
    if(Game.getLocalPlayer() == nullptr)
        return;

    // Get the target block from Baritone
    Baritone* baritone = ModuleManager::getModule<Baritone>();
    if(baritone && baritone->hasTarget) {
        mineType = baritone->bmineType;
        multiTask = baritone->bmultiTask;
        switchBack = baritone->bswitchBack;
        mineRange = baritone->bmineRange;
        continueReset = baritone->bcontinueReset;
        mineSpeed = baritone->pckminespeed;
    }

    if(localPlayer->getItemUseDuration() > 0 && multiTask == 1) {
        gm->destroyProgress = 0.f;
        breakPair = std::make_pair(BlockPos(0, 0, 0), -1);
        return;
    }
    if(!canBreakBlock(breakPair.first))
        return;
    if(switchBack && shouldSwitch && lastSlot != -1) {
        SpoofSwitchezels(lastSlot);
        lastSlot = -1;
    }
    std::pair<float, uint8_t> bestSlot = getMineTool(breakPair.first);
    if(gm->destroyProgress < 1.f) {
        gm->destroyProgress += bestSlot.first * mineSpeed;
        if(mineType == 2)
            gm->destroyProgress = 1.f;
        if(gm->destroyProgress > 1.f)
            gm->destroyProgress = 1.f;
    } else {
        if(localPlayer->getItemUseDuration() > 0 && multiTask == 0)
            return;
        SpoofSwitchezels(bestSlot.second);
        uint8_t oldSlot = InvUtil::getSelectedSlot();
        breakBlock(breakPair.first, breakPair.second);
        if(switchBack) {
            shouldSwitch = true;
            lastSlot = oldSlot;
        }
    }
}

void PacketMine::onLevelRender() {
    LocalPlayer* localPlayer = Game.getLocalPlayer();

    BlockSource* region = Game.getClientInstance()->getRegion();
    if(region == nullptr)
        return;

    GameMode* gameMode = localPlayer->gamemode;
    if(gameMode == nullptr)
        return;



    float destroyProgress = gameMode->destroyProgress;
    if(destroyProgress > 0.f) {
        BlockPos blockLookingAt = gameMode->lastBreakPos;
        Block* block = region->getBlock(blockLookingAt);

        AABB blockAABB;
        block->blockLegacy->getOutline(block, region, &blockLookingAt, &blockAABB);

        UIColor fillColor =
            ColorUtil::lerp(UIColor(255, 0, 0), UIColor(0, 255, 0), destroyProgress);
        UIColor lineColor = fillColor;

        fillColor.a = 85;
        lineColor.a = 255;

        //DrawUtil::drawBox3dFilled(blockAABB, fillColor, lineColor, destroyProgress);
    }
}