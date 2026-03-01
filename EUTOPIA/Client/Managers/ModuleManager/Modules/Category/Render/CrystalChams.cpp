#include "CrystalChams.h"

#include "../../../../../../Libs/glm/glm/glm.hpp"
#include "../../../../../../Utils/ColorUtil.h"
#include "../../../../../../Utils/Minecraft/TargetUtil.h"
#include <DrawUtil.h>
#include <cmath>

CrystalChams::CrystalChams() : Module("CrystalChams", "CrystalChams By Aqua", Category::RENDER) {
    // Basic toggles
    registerSetting(new BoolSetting("Fill", "Render filled box", &fill, fill));

    // Color & speed
    registerSetting(new ColorSetting("Color", "Box color", &color, color, true));
    registerSetting(new SliderSetting<float>("Speed", "Rotation speed", &speed, speed, 0.1f, 5.f));

    // Other sliders
    registerSetting(new SliderSetting<float>("Scale", "Box scale", &scale, scale, 0.1f, 2.f));
    registerSetting(new SliderSetting<float>("Range", "Render distance", &range, range, 0.f, 32.f));

    registerSetting(new SliderSetting<int>("LineAlpha", "Line alpha", &lineAlpha, lineAlpha, 0, 255));

    // Vertical movement sliders
    registerSetting(new SliderSetting<float>("VSpeed", "Vertical movement speed", &upDownSpeed, upDownSpeed, 0.1f, 5.f));
    registerSetting(new SliderSetting<float>("Height", "Vertical movement amplitude", &heightOffset, heightOffset, 0.1f, 4.f));
}

void CrystalChams::onLevelRender() {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;

    Level* level = localPlayer->level;
    if(!level)
        return;

    static float timeAcc = 0.f;
    timeAcc += DrawUtil::deltaTime;

    for(auto& entity : localPlayer->level->getRuntimeActorList()) {
        if(!entity)
            continue;
        if(entity->getActorTypeComponent()->id != ActorType::EnderCrystal)
            continue;

        AABBShapeComponent* aabbShape = entity->getAABBShapeComponent();
        if(!aabbShape)
            continue;

        Vec3<float> playerPos = localPlayer->getPos();
        Vec3<float> entityPos = entity->getPos();
        float distSq = (playerPos - entityPos).squaredlen();
        if(distSq > range * range)
            continue;

        // Calculate the AABB (no bounce effect for simplicity)
        AABB renderAABB = aabbShape->getAABB();

        // Apply vertical oscillation
        float yOffset = std::sinf(timeAcc * upDownSpeed) * heightOffset;
        renderAABB.lower.y += yOffset;
        renderAABB.upper.y += yOffset;

        float baseScale = scale;

        const Vec3<float> axes[3] = {Vec3<float>(1.f, 0.5f, 0.5f),
                                     Vec3<float>(0.5f, 1.f, 0.5f),
                                     Vec3<float>(0.5f, 0.5f, 1.f)};

        for(const auto& axis : axes) {
            float angle = timeAcc * speed;
            Vec3<float> rotVec = axis.normalize() * angle;

            UIColor fillColor = fill ? color : UIColor(color.r, color.g, color.b, 0);
            UIColor lineColor(color.r, color.g, color.b, lineAlpha);

            DrawUtil::drawBox3dFilledRotation(renderAABB, fillColor, lineColor, baseScale,
                                             DrawUtil::BoxRotation(rotVec.x, rotVec.y, rotVec.z));
        }
    }
}