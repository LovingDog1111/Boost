#include "PlayerUtil.h"

uint32_t PlayerUtil::selectedSlotServerSide = 0;

bool PlayerUtil::canPlaceBlock(const BlockPos& blockPos, bool airPlace) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    BlockSource* region = GI::getRegion();
    Block* block = region->getBlock(blockPos);
    BlockLegacy* blockLegacy = block->blockLegcy;

    if(blockLegacy->canBeBuiltOver(region, blockPos)) {
        if(airPlace) {
            return true;
        }

        BlockPos blockToPlace = blockPos;

        static BlockPos checklist[6] = {BlockPos(0, -1, 0), BlockPos(0, 1, 0),  BlockPos(0, 0, -1),
                                        BlockPos(0, 0, 1),  BlockPos(-1, 0, 0), BlockPos(1, 0, 0)};

        for(const BlockPos& current : checklist) {
            BlockPos calc = blockToPlace.sub(current);
            if(!region->getBlock(calc)->blockLegcy->canBeBuiltOver(region, calc)) {
                return true;
            }
        }
    }
    return false;
}

bool PlayerUtil::tryPlaceBlock(const BlockPos& blockPos, bool airPlace, bool packetPlace) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    BlockSource* region = GI::getRegion();
    Block* block = region->getBlock(blockPos);
    BlockLegacy* blockLegacy = block->blockLegcy;

    if(blockLegacy->canBeBuiltOver(region, blockPos)) {
        BlockPos blockToPlace = blockPos;

        static BlockPos checklist[6] = {BlockPos(0, -1, 0), BlockPos(0, 1, 0),  BlockPos(0, 0, -1),
                                        BlockPos(0, 0, 1),  BlockPos(-1, 0, 0), BlockPos(1, 0, 0)};

        bool foundCandidate = false;
        uint8_t i = 0;

        for(const BlockPos& current : checklist) {
            BlockPos calc = blockPos.add(current); // compute adjacent block based on original position
            if(!region->getBlock(calc)->blockLegcy->canBeBuiltOver(region, calc)) {
                foundCandidate = true;
                blockToPlace = calc;
                break;
            }
            i++;
        }

        if(airPlace && !foundCandidate) {
            foundCandidate = true;
            blockToPlace = blockPos;
            i = 0;
        }

        if(foundCandidate) {
            if(!packetPlace) {
                localPlayer->gamemode->buildBlock(blockToPlace, i, false);
            } else {
                PlayerInventory* plrInv = localPlayer->supplies;
                auto itemUseInvTransac = std::make_unique<ItemUseInventoryTransaction>();
                itemUseInvTransac->mActionType = ItemUseInventoryTransaction::ActionType::Place;
                itemUseInvTransac->mBlockPos = blockToPlace;
                itemUseInvTransac->mTargetBlockRuntimeId =
                    *region->getBlock(blockToPlace)->getRuntimeID();
                itemUseInvTransac->mFace = static_cast<uint32_t>(i);
                itemUseInvTransac->mSlot = static_cast<uint32_t>(plrInv->mSelectedSlot);
                ItemStack* handItem = plrInv->container->getItem(plrInv->mSelectedSlot);
                if(handItem)
                    itemUseInvTransac->mItemInHand = NetworkItemStackDescriptor(*handItem);
                itemUseInvTransac->mPlayerPos = localPlayer->getPos();
                itemUseInvTransac->mClickPos = Vec3<float>(0.5f, 0.5f, 0.5f);

                InventoryTransactionPacket pk;
                pk.mTransaction = std::move(itemUseInvTransac);
               GI::getPacketSender()->send(&pk);
            }
            return true;
        }
    }
    return false;
}