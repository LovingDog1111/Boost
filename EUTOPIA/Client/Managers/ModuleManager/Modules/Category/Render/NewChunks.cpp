#include "NewChunks.h"
#include <DrawUtil.h>
#include "../../../../../../SDK/Render/MeshHelpers.h"
NewChunks::NewChunks()
    : Module("NewChunks", "Highlights newly generated chunks", Category::RENDER) {
    registerSetting(new SliderSetting<float>("Y Height", "NULL", &yHeight, -64, -64, 0));
    registerSetting(new BoolSetting(
        "Detect Flowing", "Include flowing water/lava in new chunk checks", &detectFlowing, false));
}

std::string NewChunks::getModeText() {
    return std::to_string(chunkCount);
}
bool worldToScreen(const Vec3<float>& pos, Vec2<float>& out) {
    ClientHMDState* hmdState = GI::getClientInstance()->getClientHMDState();

    Vec2<float> screenSize = GI::getClientInstance()->guiData->windowSizeScaled;
    Vec3<float> relativePos = pos.sub(Game.getClientInstance()->getLevelRenderer()->renderplayer->origin);

    glm::mat4 mvpMatrix = hmdState->lastProjectionMatrix * hmdState->lastviewMatrix;
    glm::vec4 clipSpacePosition =
        mvpMatrix * glm::vec4(relativePos.x, relativePos.y, relativePos.z, 1.0f);

    if(clipSpacePosition.w <= 0.0f)
        return false;

    clipSpacePosition /= clipSpacePosition.w;

    out = Vec2<float>((clipSpacePosition.x + 1.0f) * (0.5f * screenSize.x),
                      (1.0f - clipSpacePosition.y) * (0.5f * screenSize.y));

    return true;
}
void NewChunks::renderNewChunk(const ChunkPos& chunkPos) {
    Vec3<float> point3D[4];
    point3D[0] = Vec3<float>((float)chunkPos.x, yHeight, (float)chunkPos.z);
    point3D[1] = Vec3<float>((float)chunkPos.x + 16.f, yHeight, (float)chunkPos.z);
    point3D[2] = Vec3<float>((float)chunkPos.x + 16.f, yHeight, (float)chunkPos.z + 16.f);
    point3D[3] = Vec3<float>((float)chunkPos.x, yHeight, (float)chunkPos.z + 16.f);

    Vec2<float> point2D[4];
    for(int i = 0; i < 4; i++) {
        if(!worldToScreen(point3D[i], point2D[i]))
            return;
    }

    for(int i = 0; i < 4; i++) {
        DrawUtil::tessellator->vertex(point2D[i].x, point2D[i].y, 0.f);
    }

    for(int i = 3; i >= 0; i--) {
        DrawUtil::tessellator->vertex(point2D[i].x, point2D[i].y, 0.f);
    }

    for(int i = 0; i < 4; i++) {
        Vec2<float> currentPoint = point2D[i];
        Vec2<float> nextPoint = point2D[(i + 1) % 4];
        size_t hash = std::hash<float>()(currentPoint.x) ^ std::hash<float>()(currentPoint.y) ^
                      std::hash<float>()(nextPoint.x) ^ std::hash<float>()(nextPoint.y);

        lineRenderList[hash] = !lineRenderList[hash];
    }
}


void NewChunks::onMCRender(MinecraftUIRenderContext* renderCtx) {
    chunkCount = chunkMap.size();
    if(!GI::canUseMoveKeys())
        return;

    if(chunkMap.empty())
        return;

    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(localPlayer == nullptr)
        return;

    Vec3<float> lpPos = localPlayer->getPos();
    Vec3<float> lpPos2D = Vec3<float>(lpPos.x, 0.f, lpPos.z);

    for(auto it = chunkMap.begin(); it != chunkMap.end();) {
        const ChunkPos& chunkPos = it->second.value();
        float distance = lpPos2D.dist(Vec3<float>(chunkPos.x + 8.5f, 0.f, chunkPos.z + 8.5f));
        if(distance < 256.f) {
            it++;
        } else {
            it = chunkMap.erase(it);
        }
    }

    lineRenderList.clear();

    DrawUtil::setColor(mce::Color(1.f, 1.f, 1.f, 1.f));
    DrawUtil::tessellator->begin(VertextFormat::QUAD);
    DrawUtil::tessellator->color(1.f, 0.f, 0.f, 0.35f);
    for(auto& it : chunkMap) {
        const ChunkPos& chunkPos = it.second.value();
        renderNewChunk(chunkPos);
    }
    MeshHelpers::renderMeshImmediately(DrawUtil::screenCtx, DrawUtil::tessellator,
                                       DrawUtil::uiMaterial);

    DrawUtil::tessellator->begin(VertextFormat::LINE_LIST);
    DrawUtil::tessellator->color(1.f, 0.f, 0.f, 1.f);
    for(auto& it : chunkMap) {
        const ChunkPos& chunkPos = it.second.value();
        DrawUtil::renderNewChunkOutline(chunkPos, yHeight, lineRenderList);
    }
    MeshHelpers::renderMeshImmediately(DrawUtil::screenCtx, DrawUtil::tessellator,
                                       DrawUtil::uiMaterial);
}