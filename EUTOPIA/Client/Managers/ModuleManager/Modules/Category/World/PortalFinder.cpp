#include "PortalFinder.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <unordered_map>
#undef min
#undef max

#include "../SDK/Render/MeshHelpers.h"
#include "../Utils/DrawUtil.h"

PortalFinder::PortalFinder()
    : Module("PortalFinder", "Find Nether Portals and show coordinates", Category::WORLD) {
    registerSetting(
        new SliderSetting<int>("Range", "Search range for Nether Portals", &range, 200, 10, 300));
    registerSetting(
        new BoolSetting("Tracer", "Draw tracer to found portals", &tracerSetting, true));
}

void PortalFinder::onEnable() {
    LocalPlayer* localPlayer = Game.getLocalPlayer();
    if(!localPlayer)
        return;

    BlockSource* region = GI::getClientInstance()->getRegion();
    if(!region)
        return;

    foundPortals.clear();
    Vec3<float> playerPos = localPlayer->getPos();
    BlockPos base((int)playerPos.x, (int)playerPos.y, (int)playerPos.z);

    int minX = base.x - range;
    int maxX = base.x + range;
    int minY = (std::max)((int)base.y - 50, 0);
    int maxY = (std::min)((int)base.y + 50, 255);
    int minZ = base.z - range;
    int maxZ = base.z + range;

    bool foundAny = false;

    for(int x = minX; x <= maxX; x++) {
        for(int y = minY; y <= maxY; y++) {
            for(int z = minZ; z <= maxZ; z++) {
                BlockPos bp(x, y, z);
                Block* block = region->getBlock(bp);
                if(!block || !block->blockLegacy)
                    continue;

                if(block->blockLegacy->blockId == 90) {
                    foundAny = true;
                    Vec3<float> portalPos = bp.CastTo<float>().add2(0.5f, 0.5f, 0.5f);
                    foundPortals.push_back(portalPos);

                    char buf[256];
                    snprintf(buf, sizeof(buf),
                             "[PortalFinder] Found Nether Portal at X%s%d%s Y%s%d%s Z%s%d%s",
                             MCTF::AQUA, x, MCTF::WHITE, MCTF::GREEN, y, MCTF::WHITE, MCTF::RED, z,
                             MCTF::WHITE);
                    Game.DisplayClientMessage(buf, MCTF::WHITE);
                }
            }
        }
    }

    if(!foundAny)
        Game.DisplayClientMessage("[PortalFinder] No Nether Portals found in range", MCTF::RED);

    this->setEnabled(false);
}

void PortalFinder::onLevelRender() {
    if(!tracerSetting || foundPortals.empty())
        return;

    LocalPlayer* localPlayer = Game.getLocalPlayer();
    if(!localPlayer)
        return;

    Tessellator* tess = DrawUtil::tessellator;
    if(!tess)
        return;

    Vec3<float> origin = Game.getClientInstance()->getLevelRenderer()->renderplayer->origin;

    for(const auto& portalPos : foundPortals) {
        Vec3<float> start = {0, 0, 0};
        Vec3<float> end = portalPos.sub(origin);

        DrawUtil::setColor(UIColor(128, 0, 255, 200));  
        tess->begin(VertextFormat::LINE_LIST, 2);
        tess->vertex(start.x, start.y, start.z);
        tess->vertex(end.x, end.y, end.z);
        MeshHelpers::renderMeshImmediately(DrawUtil::screenCtx, tess, DrawUtil::blendMaterial);
    }
}
