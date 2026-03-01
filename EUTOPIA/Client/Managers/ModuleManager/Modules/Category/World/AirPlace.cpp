#include "AirPlace.h"

#include "..\\Utils\\DrawUtil.h"

AirPlace::AirPlace() : Module("AirPlace", "Place blocks on air", Category::WORLD) {
    registerSetting(new BoolSetting(
        "Render", "Renders a block overlay where your block will be placed", &render, true));
    registerSetting(new ColorSetting("Color", "NULL", &color, UIColor(0, 255, 0), false));
    registerSetting(new SliderSetting<int>("Opacity", "NULL", &opacity, 0, 0, 255));
    registerSetting(new SliderSetting<int>("LineOpacity", "NULL", &lineOpacity, 255, 0, 255));
}

void AirPlace::onNormalTick(LocalPlayer* localPlayer) {
    shouldRender = false;

    if(!localPlayer)
        return;

    ClientInstance* ci = Game.getClientInstance();
    if(!ci)
        return;

    BlockSource* region = ci->getRegion();
    if(!region)
        return;

    Level* level = localPlayer->level;
    if(!level)
        return;

    HitResult* hitResult = level->getHitResult();
    if(!hitResult)
        return;

    if(hitResult->type != HitResultType::AIR)
        return;

    ItemStack* carriedItem = localPlayer->getCarriedItem();
    if(!carriedItem)
        return;

    if(!carriedItem->isValid() || !carriedItem->isBlock())
        return;

    Vec3<float> endPos = hitResult->endPos;

    BlockPos placePos((int)floorf(endPos.x), (int)floorf(endPos.y), (int)floorf(endPos.z));

    if(Game.canUseMoveKeys() && Game.isRightClickDown()) {
        if(shouldPlace) {
            if(localPlayer->gamemode)
                localPlayer->gamemode->buildBlock(placePos, 0, true);
            shouldPlace = false;
        }
    } else {
        shouldPlace = true;
    }

    renderAABB.lower = placePos.CastTo<float>();
    renderAABB.upper = renderAABB.lower.add(Vec3<float>(1.f, 1.f, 1.f));
    shouldRender = true;
}

void AirPlace::onLevelRender() {
    if(!render || !shouldRender)
        return;

    DrawUtil::drawBox3dFilled(renderAABB, UIColor(color.r, color.g, color.b, opacity),
                              UIColor(color.r, color.g, color.b, lineOpacity));
}