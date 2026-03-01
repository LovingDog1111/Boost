#include "BlockTracer.h"

#include <algorithm>
#include <cmath>

#include "../../SDK/Render/MeshHelpers.h"

BlockTracer::BlockTracer()
    : Module("BlockTracer", "Draws a tracer to a specific block", Category::WORLD) {
    registerSetting(
        new SliderSetting<int>("Range", "Search range for target block", &range, 50, 10, 300));
}

Block* targetBlockPtr = nullptr;

void BlockTracer::setTargetBlock(int blockId) {
    targetBlockId = blockId;
    hasTarget = false;

    LocalPlayer* player = Game.getLocalPlayer();
    if(!player)
        return;

    BlockSource* region = GI::getClientInstance()->getRegion();
    if(!region)
        return;

    Vec3<float> playerPos = player->getPos();
    float closestDist = FLT_MAX;

    int minX = std::max((int)playerPos.x - range, 0);
    int maxX = (int)playerPos.x + range;
    int minY = 5;
    int maxY = 128;
    int minZ = std::max((int)playerPos.z - range, 0);
    int maxZ = (int)playerPos.z + range;

    for(int x = minX; x <= maxX; ++x) {
        for(int y = minY; y <= maxY; ++y) {
            for(int z = minZ; z <= maxZ; ++z) {
                BlockPos pos(x, y, z);
                Block* block = region->getBlock(pos);
                if(!block || !block->blockLegacy)
                    continue;
                if(block->blockLegacy->blockId != targetBlockId)
                    continue;

                Vec3<float> blockVec(x + 0.5f, y + 0.5f, z + 0.5f);
                float dx = blockVec.x - playerPos.x;
                float dy = blockVec.y - playerPos.y;
                float dz = blockVec.z - playerPos.z;
                float dist = sqrtf(dx * dx + dy * dy + dz * dz);

                if(dist < closestDist) {
                    closestDist = dist;
                    targetPos = blockVec;
                    targetBlockPtr = block;
                    hasTarget = true;
                }
            }
        }
    }

    char buf[128];
    if(hasTarget)
        snprintf(buf, sizeof(buf), "[BlockTracer] Target block ID %d found", blockId);
    else
        snprintf(buf, sizeof(buf), "[BlockTracer] No target blocks found for ID %d", blockId);
    Game.DisplayClientMessage(buf, hasTarget ? MCTF::GREEN : MCTF::RED);
}

void BlockTracer::drawTracerToBlock(const Vec3<float>& blockPos) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;

    Tessellator* tessellator = DrawUtil::tessellator;
    if(!tessellator)
        return;

    float yaw = localPlayer->getActorRotationComponent()->mYaw;
    float pitch = localPlayer->getActorRotationComponent()->mPitch;
    float calcYaw = (yaw + 90.f) * (PI / 180.f);
    float calcPitch = pitch * -(PI / 180.f);

    Vec3<float> moveVec;
    moveVec.x = cos(calcYaw) * cos(calcPitch) * 0.2f;
    moveVec.y = sin(calcPitch) * 0.2f;
    moveVec.z = sin(calcYaw) * cos(calcPitch) * 0.2f;

    Vec3<float> origin = Game.getClientInstance()->getLevelRenderer()->renderplayer->origin;
    Vec3<float> start = moveVec;
    Vec3<float> end = blockPos.sub(origin);

    DrawUtil::setColor(UIColor(255, 0, 0, 200));
    tessellator->begin(VertextFormat::LINE_LIST, 2);
    tessellator->vertex(start.x, start.y, start.z);
    tessellator->vertex(end.x, end.y, end.z);
    MeshHelpers::renderMeshImmediately(DrawUtil::screenCtx, tessellator, DrawUtil::blendMaterial);

    if(!targetBlockPtr)
        return;

    BlockSource* region = GI::getClientInstance()->getRegion();
    if(!region)
        return;

    BlockPos blockIntPos((int)targetPos.x, (int)targetPos.y, (int)targetPos.z);
    AABB blockAABB;
    targetBlockPtr->blockLegacy->getOutline(targetBlockPtr, region, &blockIntPos, &blockAABB);

    
    blockAABB.lower.x += blockIntPos.x;
    blockAABB.lower.y += blockIntPos.y;
    blockAABB.lower.z += blockIntPos.z;
    blockAABB.upper.x += blockIntPos.x;
    blockAABB.upper.y += blockIntPos.y;
    blockAABB.upper.z += blockIntPos.z;

    float destroyProgress = 0.f;
    UIColor fillColor = ColorUtil::lerp(UIColor(255, 0, 0), UIColor(0, 255, 0), destroyProgress);
    UIColor lineColor = fillColor;
    fillColor.a = 85;
    lineColor.a = 255;

    DrawUtil::drawBox3dFilled(blockAABB, fillColor, lineColor, destroyProgress);
}



void BlockTracer::onLevelRender() {
    if(hasTarget)
        drawTracerToBlock(targetPos);
}

void BlockTracer::onEnable() {
    Game.DisplayClientMessage(
        "[BlockTracer] Enabled. Run .tracer [id] again if the target updates.", MCTF::GREEN);
}
