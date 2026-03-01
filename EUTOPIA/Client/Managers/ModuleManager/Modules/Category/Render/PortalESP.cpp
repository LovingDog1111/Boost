#include "PortalESP.h"

#include <DrawUtil.h>

PortalESP::PortalESP()
    : Module("PortalESP", "Renders Portals in render distance", Category::RENDER) {
    registerSetting(new SliderSetting<int>("Radius",
                                           "The more, the slower the module scans for blocks, this "
                                           "is only for horizontal radius, it shouldnt affect fps",
                                           &radius, radius, 24, 512));
    registerSetting(new SliderSetting<int>("Alpha", "Opacity of render", &alpha, 128, 0, 255));
    registerSetting(new SliderSetting<int>("Line Alpha", "Line opacity of render", &Linealpha,
                                           Linealpha, 0, 255));
}

void PortalESP::onLevelRender() {
    Tessellator* tessellator = DrawUtil::tessellator;

    auto ci = GI::getClientInstance();
    if(!ci)
        return;

    LocalPlayer* localPlayer = ci->getLocalPlayer();
    if(!localPlayer)
        return;

    AABBShapeComponent* playerAABBShape = localPlayer->getAABBShapeComponent();
    if(!playerAABBShape)
        return;

    AABB playerAABB = playerAABBShape->getAABB();

    Vec3<float> feetPos = playerAABB.getCenter();
    feetPos.y = playerAABB.lower.y;

    const BlockPos& currentPos = feetPos.add(Vec3<float>(0.f, 0.5f, 0.f)).floor().CastTo<int>();

    for(auto it = blockList.begin(); it != blockList.end();) {
        if(it->second == UIColor(0, 0, 0, 0) || abs(currentPos.x - it->first.x) > radius ||
           abs(currentPos.z - it->first.z) > radius || abs(currentPos.y - it->first.y) > radius) {
            it = blockList.erase(it);
        } else {
            ++it;
        }
    }

    auto region = ci->getRegion();
    if(!region)
        return;

    if(!blockList.empty()) {
        for(const auto& pos : blockList) {
            BlockPos blockPos = pos.first;

            Block* block = region->getBlock(blockPos);
            if(!block)
                continue;

            BlockLegacy* blockLegacy = block->blockLegcy;
            if(!blockLegacy)
                continue;

            AABB blockAABB;
            blockLegacy->getOutline(block, region, &blockPos, &blockAABB);

            UIColor boxColor = pos.second;
            DrawUtil::drawBox3dFilled(blockAABB, UIColor(boxColor.r, boxColor.g, boxColor.b, alpha),
                                      UIColor(boxColor.r, boxColor.g, boxColor.b, Linealpha), 1.f);
        }
    }
}
