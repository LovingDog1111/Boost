#include "ItemESP.h"

#include <DrawUtil.h>

#include "..\SDK\World\Actor\ItemActor.h"
#include "..\SDK\World\Item\Item.h"
#include "..\Utils\Minecraft\InvUtil.h"

ItemESP::ItemESP() : Module("ItemESP", "Highlights items", Category::RENDER) {
    registerSetting(new BoolSetting("Show names", "Display item names", &showNames, true));
    registerSetting(new SliderSetting<float>("Distance", "The distance to show items within",
                                             &distance, 0.f, 100.f, 1.f));
    registerSetting(new BoolSetting("Render Filled", "Whether to render the boxes filled",
                                    &renderFilled, false));
    registerSetting(
        new BoolSetting("Themed Color", "Whether to use the themed color", &themedColor, true));
    registerSetting(
        new BoolSetting("Show Names", "Whether to show the item names", &showItemNames, true));
    registerSetting(new BoolSetting("Distance Scaled Font",
                                    "Whether to scale the font based on distance",
                                    &distanceScaledFont, true));
    registerSetting(
        new SliderSetting<float>("Font Size", "The size of the font", &fontSize, 1.f, 40.f, 0.01f));
    registerSetting(new SliderSetting<float>("Scaling Multiplier",
                                             "The multiplier to use for scaling the font",
                                             &scalingMultiplier, 0.f, 5.f, 0.01f));
    registerSetting(new BoolSetting("Highlight Useful Items", "Toggle highlighting of useful items",
                                    &highlightUsefulItems, true));
    registerSetting(new BoolSetting("Show Enchant", "Whether to show the enchantments of the items",
                                    &showEnchant, true));
}

void ItemESP::onNormalTick(LocalPlayer* player) {
    auto playerPtr = GI::getLocalPlayer();
    std::lock_guard<std::mutex> lock(itemMutex);

    items.clear();
    auto actors = ActorUtils::getActorsTyped<ItemActor>(ActorType::ItemEntity);

    for(auto actor : actors) {
        if(!actor || !actor->mItem.mItem || !actor->getStateVectorComponent())
            continue;

        Vec3<float> actorPos = actor->getRenderPositionComponent()->mPosition;
        Vec3<float> playerPos = playerPtr->getPos();
        float dx = playerPos.x - actorPos.x;
        float dy = playerPos.y - actorPos.y;
        float dz = playerPos.z - actorPos.z;
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);

        if(distance > 0.f && dist > distance)
            continue;

        auto renderPosComp = actor->getRenderPositionComponent();
        if(!renderPosComp)
            continue;
        auto shape = actor->getAABBShapeComponent();
        if(!shape)
            continue;

        Vec3<float> pos = renderPosComp->mPosition;
        float aabbHeight = shape->mHeight;
        float aabbWidth = shape->mWidth;

        ItemStack* stack = &actor->mItem;
        if(!stack->mItem)
            continue;

        std::string name = stack->getItem()->mName;
        if(name.empty())
            continue;
        name += " x" + std::to_string(stack->mCount);

        std::vector<std::pair<std::string, int>> enchants;
        for(int i = 0; i <= static_cast<int>(Enchant::SWIFT_SNEAK); ++i) {
            int enchantValue = stack->getEnchantValue(i);
            if(enchantValue > 0)
                enchants.emplace_back(stack->getEnchantName(static_cast<Enchant>(i)), enchantValue);
        }

        items.emplace_back(pos, Vec2<float>(aabbWidth, aabbHeight), name, enchants);
    }
}

void ItemESP::onLevelRender() {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;

    std::lock_guard<std::mutex> lock(itemMutex);

    for(auto& actorInfo : items) {
        Vec3<float> pos = actorInfo.pos;
        Vec2<float> size = actorInfo.size;

        AABB box;
        box.lower = Vec3<float>(pos.x - size.x / 2.0f, pos.y, pos.z - size.x / 2.0f);
        box.upper = Vec3<float>(pos.x + size.x / 2.0f, pos.y + size.y, pos.z + size.x / 2.0f);
        box.size = Vec2<float>(box.upper.x - box.lower.x, box.upper.y - box.lower.y);

        UIColor boxColor(1.0f, 1.0f, 1.0f, 0.25f);
        if(themedColor)
            boxColor = ColorUtil::getAquaThemeColor(0);

        if(renderFilled)
            DrawUtil::drawBox3dFilled(box, boxColor, 0.25f);

        DrawUtil::drawBox3dFilled(box, boxColor, 2.0f);

        if(!showItemNames)
            continue;

        std::string name = actorInfo.name;
        if(showEnchant && !actorInfo.enchants.empty()) {
            std::string enchantText;
            for(auto& enchant : actorInfo.enchants) {
                if(!enchantText.empty())
                    enchantText += ", ";
                enchantText += "[" + enchant.first + "]";
            }
            name += " " + enchantText;
        }

        Vec3<float> textPos3D(pos.x, pos.y + size.y + 0.5f, pos.z);
        Vec2<float> screenPos(0.0f, 0.0f);
        RenderUtil::WorldToScreen(textPos3D, screenPos);

        float fontScale = fontSize;
        if(distanceScaledFont) {
            Vec3<float> playerPos = localPlayer->getPos();
            float dx = playerPos.x - pos.x;
            float dy = playerPos.y - pos.y;
            float dz = playerPos.z - pos.z;
            float dist = sqrtf(dx * dx + dy * dy + dz * dz) + 2.5f;
            if(dist < 0.0f)
                dist = 0.0f;
            fontScale = (1.0f / dist) * 100.0f * scalingMultiplier;
            if(fontScale < 1.0f)
                fontScale = 1.0f;
        }

        UIColor textColor = highlightUsefulItems ? UIColor(0.0f, 1.0f, 0.0f, 1.0f)
                                                 : UIColor(1.0f, 1.0f, 1.0f, 1.0f);
        DrawUtil::drawText(screenPos, name, textColor, fontScale);
    }
}
