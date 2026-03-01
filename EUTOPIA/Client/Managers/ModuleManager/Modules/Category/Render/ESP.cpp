#include "ESP.h"

#include <DrawUtil.h>
#include <Minecraft/TargetUtil.h>

#include "..\SDK\World\Actor\ItemActor.h"
#include "..\SDK\World\Item\Item.h"
#include "..\Utils\Minecraft\InvUtil.h"

ESP::ESP() : Module("ESP", "Highlights player/entity", Category::RENDER) {
    registerSetting(new ColorSetting("Color", "NULL", &color, UIColor(0, 0, 255), false));
    registerSetting(new SliderSetting<int>("Alpha", "NULL", &alpha, 40, 0, 255));
    registerSetting(new SliderSetting<int>("LineAlpha", "NULL", &lineAlpha, 135, 0, 255));
    registerSetting(new BoolSetting("Mobs", "NULL", &mobs, false));
    registerSetting(new BoolSetting("ItemESP", "Highlight items", &itemESP, false));
}

void ESP::onLevelRender() {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;

    for(auto& entity : localPlayer->level->getRuntimeActorList()) {
        if(TargetUtil::isTargetValid(entity, mobs)) {
            AABBShapeComponent* entityAABBShape = entity->getAABBShapeComponent();
            if(entityAABBShape) {
                AABB box = entityAABBShape->getAABB();
                DrawUtil::drawBox3dFilled(box, UIColor(color.r, color.g, color.b, alpha),
                                          UIColor(color.r, color.g, color.b, lineAlpha));
            }
        }
    }

    if(itemESP) {
        auto actors = ActorUtils::getActorsTyped<ItemActor>(ActorType::ItemEntity);
        for(auto actor : actors) {
            if(!actor || !actor->mItem.mItem || !actor->getStateVectorComponent())
                continue;

            auto renderPosComp = actor->getRenderPositionComponent();
            auto shape = actor->getAABBShapeComponent();
            if(!renderPosComp || !shape)
                continue;

            Vec3<float> pos = renderPosComp->mPosition;
            float width = shape->mWidth;
            float height = shape->mHeight;

            AABB box;
            box.lower = Vec3<float>(pos.x - width / 2.0f, pos.y, pos.z - width / 2.0f);
            box.upper = Vec3<float>(pos.x + width / 2.0f, pos.y + height, pos.z + width / 2.0f);

            DrawUtil::drawBox3dFilled(box, UIColor(color.r, color.g, color.b, alpha),
                                      UIColor(color.r, color.g, color.b, lineAlpha));

            ItemStack* stack = &actor->mItem;
            if(!stack->mItem)
                continue;

            std::string name = stack->getItem()->mName;
            if(name.empty())
                continue;
            name += " x" + std::to_string(stack->mCount);

            Vec3<float> textPos3D(pos.x, pos.y + height + 0.5f, pos.z);
            Vec2<float> screenPos(0.0f, 0.0f);
            RenderUtil::WorldToScreen(textPos3D, screenPos);

            DrawUtil::drawText(screenPos, name, UIColor(color.r, color.g, color.b, 255), 1.0f);



        }
    }
}
