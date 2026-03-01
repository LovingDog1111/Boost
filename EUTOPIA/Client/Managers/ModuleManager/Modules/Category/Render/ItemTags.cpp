#include "ItemTags.h"

#include <mutex>
#include <vector>

#include "../../../../../../SDK/Render/Matrix.h"
#include "..\SDK\World\Actor\ItemActor.h"
#include "..\Utils\Minecraft\InvUtil.h"

std::mutex itemMutex;

ItemTags::ItemTags() : Module("ItemTags", "Displays item names", Category::RENDER) {
    registerSetting(new SliderSetting<float>("Size", "Tag size", &tagSize, 0.5f, 0.f, 1.f));
    registerSetting(new SliderSetting<float>("Opacity", "Opacity", &opacity, 1.f, 0.f, 1.f));
    registerSetting(new BoolSetting("Dynamic Size", "Scale with distance", &dynamicSize, true));
}

void ItemTags::onNormalTick(LocalPlayer* player) {
    LocalPlayer* lp = GI::getLocalPlayer();
    if(!lp)
        return;

    std::lock_guard<std::mutex> lock(itemMutex);

    auto actors = ActorUtils::getActorsTyped<ItemActor>(ActorType::ItemEntity);
    for(auto actor : actors) {
        if(!actor)
            continue;
        if(!actor->mItem.mItem)
            continue;
        if(!actor->getStateVectorComponent())
            continue;

        auto renderPosComp = actor->getRenderPositionComponent();
        if(!renderPosComp)
            continue;

        ItemStack* stack = &actor->mItem;
        if(!stack->mItem || !stack->getItem())
            continue;

        std::string name = stack->getItem()->mName;
        if(name.empty())
            name = "Unknown Item";
        name += " x" + std::to_string(stack->mCount);

        Vec3<float> pos = renderPosComp->mPosition;

        Vec2<float> screenPos;
        if(!Matrix::WorldToScreen(pos, screenPos))
            continue;

        float distance = lp->getPos().dist(pos);
        float scale = tagSize;
        if(dynamicSize) {
            if(distance <= 1.f)
                scale = tagSize * 3.f;
            else if(distance >= 4.f)
                scale = tagSize;
            else
                scale = tagSize * (3.f - ((distance - 1.f) * (2.f / 3.f)));
        }

        UIColor tagColor(255, 255, 255, static_cast<int>(255 * opacity));
        float textWidth = RenderUtil::getTextWidth(name, scale);
        float textHeight = RenderUtil::getTextHeight(name, scale);
        Vec2<float> textPos(screenPos.x - textWidth / 2.f, screenPos.y - textHeight / 2.f);
        RenderUtil::drawText(textPos, name, tagColor, scale, true);
    }
}

void ItemTags::onD2DRender() {}
