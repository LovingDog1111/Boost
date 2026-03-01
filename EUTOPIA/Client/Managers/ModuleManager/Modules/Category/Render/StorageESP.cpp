#include "StorageESP.h"
#include <DrawUtil.h>
#include "../../../../../../SDK/Render/MeshHelpers.h"

int Linealpha = 256;
int renderMode = 2;  // 0 = Tracer, 1 = Box, 2 = Both

StorageESP::StorageESP()
    : Module("ChestVisuals", "Renders storages in render distance", Category::RENDER) {
    registerSetting(
        new SliderSetting<int>("Radius", "Scanning radius for blocks", &radius, radius, 24, 512));
    registerSetting(new SliderSetting<int>("Alpha", "Opacity of render", &alpha, 128, 0, 255));
    registerSetting(new SliderSetting<int>("Line Alpha", "Line opacity of render", &Linealpha,
                                           Linealpha, 0, 255));
    registerSetting(new BoolSetting("Chests", "Render normal chests", &chest, chest));
    registerSetting(
        new BoolSetting("Ender Chests", "Render ender chests", &enderChest, enderChest));
    registerSetting(new BoolSetting("Shulkers", "Render shulker boxes", &shulkerBox, shulkerBox));
    registerSetting(new BoolSetting("Barrels", "Render barrels", &barrel, barrel));
    registerSetting(new BoolSetting("Hoppers", "Render hoppers", &hopper, hopper));
    registerSetting(new BoolSetting("Furnaces", "Render furnaces", &furnace, furnace));
    registerSetting(new BoolSetting("Spawners", "Render spawners", &spawner, spawner));
    registerSetting(new BoolSetting("Bookshelves", "Render bookshelves", &bookshelf, bookshelf));
    registerSetting(new BoolSetting("Enchant Table", "Render enchanting tables", &enchantingTable, enchantingTable));
    registerSetting(new BoolSetting("Decorated Pots", "Render decorated pots", &decoratedPot, decoratedPot));
    registerSetting(new BoolSetting("Cobwebs", "Render cobwebs", &web, web));
    registerSetting(new BoolSetting("Droppers", "Render droppers", &dropper, dropper));
    registerSetting(
        new BoolSetting("Dispensers", "Render dispensers", &dispenser, dispenser));
    registerSetting(new BoolSetting("Anvils", "Render anvils", &anvil, anvil));
    registerSetting(new EnumSetting("Render Mode", "Choose how the storage blocks are rendered",
                                    {"Tracer", "Box", "Both"}, &renderMode, 2));
}

void StorageESP::onLevelRender() {
    LocalPlayer* localPlayer = Game.getLocalPlayer();
    if(!localPlayer)
        return;

    Vec3<float> feetPos = localPlayer->getAABB(true).getCenter();
    feetPos.y = localPlayer->getAABB(true).lower.y;
    BlockPos currentPos = feetPos.add(Vec3<float>(0.f, 0.5f, 0.f)).floor().CastTo<int>();

    for(auto it = blockList.begin(); it != blockList.end();) {
        BlockPos pos = it->first;

        if(abs(currentPos.x - pos.x) > radius || abs(currentPos.z - pos.z) > radius) {
            it = blockList.erase(it);
            continue;
        }

        Block* block = Game.getClientInstance()->getRegion()->getBlock(pos);
        if(!block || !block->blockLegacy || block->blockLegacy->blockId == 0) {
            it = blockList.erase(it);
            continue;
        }

        int blockId = block->blockLegacy->blockId;
        std::string name = block->blockLegacy->mNameSpace;

        bool valid = (chest && (blockId == 54 || blockId == 146)) ||
                     (enderChest && blockId == 130) ||
                     (shulkerBox && blockId == 205) ||
                     (barrel && blockId == 458) || (hopper && blockId == 154) ||
                     (furnace && (blockId == 61 || blockId == 451 || blockId == 453)) ||
                     (spawner && blockId == 52) || (bookshelf && blockId == 47) ||
                     (enchantingTable && blockId == 116) ||
                     (decoratedPot && blockId == 806) ||
                     (web && name.find("web") != std::string::npos) ||
                     (dropper && name.find("dropper") != std::string::npos) ||
                     (dispenser && blockId == 23) || (anvil && blockId == 145);

        if(!valid) {
            it = blockList.erase(it);
        } else {
            ++it;
        }
    }

    if(!blockList.empty()) {
        for(const auto& pos : blockList) {
            BlockPos blockPos = pos.first;
            Block* block = Game.getClientInstance()->getRegion()->getBlock(blockPos);
            if(!block)
                continue;

            AABB blockAABB;
            block->blockLegacy->getOutline(block, Game.getClientInstance()->getRegion(), &blockPos,
                                           &blockAABB);

            Vec3<float> origin =
                Game.getClientInstance()->getLevelRenderer()->renderplayer->origin;
            float calcYaw = (localPlayer->getActorRotationComponent()->mYaw+ 90.f) * (PI / 180.f);
            float calcPitch =
                (localPlayer->getActorRotationComponent()->mPitch) * -(PI / 180.f);

            Vec3<float> moveVec{cos(calcYaw) * cos(calcPitch) * 0.2f, sin(calcPitch) * 0.2f,
                                sin(calcYaw) * cos(calcPitch) * 0.2f};

            Vec3<float> start = moveVec;
            Vec3<float> end = blockPos.toFloat().add2(0.5, 0.5, 0.5).sub(origin);
            UIColor boxColor = pos.second;

            if(renderMode == 0 || renderMode == 2) {  // Tracer
                DrawUtil::setColor(UIColor(boxColor.r, boxColor.g, boxColor.b, Linealpha));
                DrawUtil::tessellator->begin(VertextFormat::LINE_LIST, 2);
                DrawUtil::tessellator->vertex(start.x, start.y, start.z);
                DrawUtil::tessellator->vertex(end.x, end.y, end.z);
                MeshHelpers::renderMeshImmediately(DrawUtil::screenCtx, DrawUtil::tessellator,
                                                   DrawUtil::blendMaterial);
            }

            if(renderMode == 1 || renderMode == 2) {  // Box
                DrawUtil::drawBox3dFilled(
                    blockAABB, UIColor(boxColor.r, boxColor.g, boxColor.b, alpha),
                                     UIColor(boxColor.r, boxColor.g, boxColor.b, Linealpha), 1.f);
            }
        }
    }
}