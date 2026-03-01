#include "WorldUtil.h"
#include "../../SDK/GlobalInstance.h"
#include "../../SDK/NetWork/Packets/PlayActionPacket.h"
// -----------------------------------------------------------------------------
//  Implementation
// -----------------------------------------------------------------------------
Block* WorldUtil::getBlock(const BlockPos& blockPos) {
    return GI::getRegion()->getBlock(blockPos);
}

float WorldUtil ::distanceToPoint(const Vec3<float>& pos1, const Vec3<float>& pos2) {
    return pos1.dist(pos2);
}

float WorldUtil::getSeenPercent(const Vec3<float>& pos, const AABB& aabb) {
    return GI::getRegion()->getSeenPercent(pos, aabb);
}

float WorldUtil::getSeenPercent(const Vec3<float>& pos, Actor* actor) {
    return getSeenPercent(pos, actor->getAABBShapeComponent()->getAABB());
}


float WorldUtil::distanceToEntity(const Vec3<float>& pos, Actor* entity) {
    if(entity == nullptr) return 0.f;
    return distanceToPoint(pos, entity->getPos());
}
bool WorldUtil::canBuildOn(const BlockPos& blockPos) {
    ClientInstance* ci = GI::getClientInstance();
    if(!ci)
        return false;
    return getBlock(blockPos)->getblockLegcy()->canBeBuiltOver(ci->getRegion(),
                                                               blockPos);
}
float WorldUtil::distanceToBlock(const Vec3<float>& pos, const BlockPos& blockPos) {
    return distanceToPoint(pos, blockPos.CastTo<float>().add(Vec3<float>(0.5f, 0.5f, 0.5f)));
}

bool WorldUtil::isValidPlacePos(Vec3<int> blockPos) {
    return getBlockPlaceFace(blockPos) != -1;
}

int WorldUtil::getBlockPlaceFace(Vec3<int> blockPos) {
    for(auto& [face, offset] : blockFaceOffsets)
        if(!isAirBlock(blockPos + Vec3<int>(offset)))
            return face;
    return -1;
}

void WorldUtil::placeBlock(Vec3<int> pos, int side) {
    auto player = GI::getClientInstance()->getLocalPlayer();
     Vec3<int> blockPos = pos;
    if(side == -1)
        side = getBlockPlaceFace(blockPos);

   Vec3<int> vec = blockPos;

    if(side != -1)
        vec.add (blockFaceOffsets[side] * 0.5f);

    HitResult* res = player->level->getHitResult();

    vec .add( blockFaceOffsets[side] * 0.5f);

    res->blockPos = vec;
    res->selectedFace = side;

    res->type = HitResultType::BLOCK;
    res->indirectHit = false;
    res->rayDirection = vec.CastTo<float>();
    res->endPos = blockPos.CastTo<float>();

    bool oldSwinging = player->swinging;
    int oldSwingProgress = player->getSwingProgress();
    Vec3<int> realPos = blockPos + Vec3<int>(blockFaceOffsets[side]);

    bool sb = getBlock(realPos)->isInteractiveBlock();
    player->gamemode->buildBlock(realPos, side, true);
    player->setSwinging(oldSwinging);
    player->setSwingProgress(oldSwingProgress);

    vec.add(blockFaceOffsets[side] * 0.5f);

    res->blockPos = vec;
    res->selectedFace = side;

    res->type = HitResultType::BLOCK;
    res->indirectHit = false;
    res->rayDirection = vec.CastTo<float>();
    res->endPos = blockPos.CastTo<float>();
}



bool WorldUtil::isAirBlock(Vec3<int> blockPos) {
    auto player = GI::getClientInstance()->getLocalPlayer();
    if(!player)
        return false;
    int blockId =
        GI::getClientInstance()->getRegion()->getBlock(blockPos)->blockLegcy->blockid;
    bool isLiquid = 8 <= blockId && blockId <= 11;
    return blockId == 0 || isLiquid;
}

int WorldUtil::getExposedFace( Vec3<int> blockPos, bool useFilter) {
    static std::vector< Vec3<int>> offsetList = {
         Vec3<int>(0, -1, 0),  Vec3<int>(0, 1, 0),   Vec3<int>(0, 0, -1),
         Vec3<int>(0, 0, 1),   Vec3<int>(-1, 0, 0),  Vec3<int>(1, 0, 0),
    };
    for(int i = 0; i < offsetList.size(); i++) {
         Vec3<int> checkPos = blockPos + offsetList[i];
        if(isAirBlock(checkPos))
            return i;
        if(useFilter) {
            if(!GI::getRegion()->getBlock(checkPos)->blockLegcy->mSolid)
                return i;
        }
    }
    return -1;
}

void WorldUtil::startDestroyBlock(Vec3<int> pos, int side) {
    auto player = GI::getClientInstance()->getLocalPlayer();
     Vec3<int> blockPos = pos;
    if(side == -1)
         side = getExposedFace(blockPos,false);

   Vec3<int> vec = blockPos;


    if(side != -1)
       vec.add(blockFaceOffsets[side] * 0.5f);

   HitResult* res = player->level->getHitResult();

   vec.add(blockFaceOffsets[side] * 0.5f);

   res->blockPos = vec;
   res->selectedFace = side;

   res->type = HitResultType::BLOCK;
   res->indirectHit = false;
   res->rayDirection = vec.CastTo<float>();
   res->endPos = blockPos.CastTo<float>();

    bool isDestroyedOut = false;
    player->gamemode->startDestroyBlock(blockPos, side, isDestroyedOut);
    // if (!isDestroyedOut) player->getGameMode()->continueDestroyBlock(blockPos, side,
    // *player->getPos(), isDestroyedOut);


    vec.add(blockFaceOffsets[side] * 0.5f);

    res->blockPos = vec;
    res->selectedFace = side;

    res->type = HitResultType::BLOCK;
    res->indirectHit = false;
    res->rayDirection = vec.CastTo<float>();
    res->endPos = blockPos.CastTo<float>();
}



void WorldUtil::destroyBlock( Vec3<int> pos, int side, bool useTransac) {
    auto player = GI::getLocalPlayer();
     Vec3<int> blockPos = pos;
    if(side == -1)
        side = getBlockPlaceFace(blockPos);

    if(!useTransac) {
        bool oldSwinging = player->swinging;
        int oldSwingProgress = player->getSwingProgress();

        player->gamemode->destroyBlock(blockPos, side);
        player->gamemode->stopDestroyBlock(blockPos);

        player->setSwinging(oldSwinging);
        player->setSwingProgress(oldSwingProgress);
        return;
    }

    auto actionPkt = MinecraftPacket::createPacket<PlayerActionPacket>();
    actionPkt->mPos = blockPos;
    actionPkt->mResultPos = blockPos;
    actionPkt->mFace = side;
    actionPkt->mAction = PlayerActionType::StopDestroyBlock;
    actionPkt->mRuntimeId = player->getRuntimeID();
    actionPkt->mtIsFromServerPlayerMovementSystem = false;

    //PacketUtils::queueSend(actionPkt, false);
    GI::getPacketSender()->sendToServer(actionPkt.get());
    auto pkt = MinecraftPacket::createPacket<InventoryTransactionPacket>();

    auto cit = std::make_unique<ItemUseInventoryTransaction>();
    cit->mActionType = ItemUseInventoryTransaction::ActionType::Destroy;
    int slot = player->supplies->mSelectedSlot;
    cit->mSlot = slot;
    cit->mItemInHand =
        NetworkItemStackDescriptor(*player->getsupplies()->getcontainer()->getItem(slot));
    cit->mBlockPos = blockPos;
    cit->mFace = side;
    cit->mTargetBlockRuntimeId = 0x92;
    cit->mPlayerPos = player->getPos();

    cit->mClickPos =  Vec3<int>(0, 0, 0).CastTo<float>();  // this is correct, i actually checked it this time
    pkt->mTransaction = std::move(cit);
    //PacketUtils::queueSend(pkt, false);
    GI::getPacketSender()->sendToServer(pkt.get());
}