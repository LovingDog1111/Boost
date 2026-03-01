#include "DihESP.h"

#include <DrawUtil.h>
#include <Minecraft/TargetUtil.h>

#include <cmath>

bool selfESP;

DihESP::DihESP() : Module("DihESP", "Life is like a dih", Category::RENDER) {
    registerSetting(new SliderSetting<int>("Alpha", "NULL", &alpha, 200, 0, 255));
    registerSetting(new SliderSetting<float>("Speed", "NULL", &speed, 2.0f, 0.1f, 10.0f));
    registerSetting(new BoolSetting("SelfESP", "Show on self", &selfESP, true));
}

void DihESP::onLevelRender() {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;

    static float t = 0.0f;
    t += speed * 0.016f;

    int r = (int)((std::sin(t) * 0.5f + 0.5f) * 255.0f);
    int g = (int)((std::sin(t + 2.094f) * 0.5f + 0.5f) * 255.0f);
    int b = (int)((std::sin(t + 4.188f) * 0.5f + 0.5f) * 255.0f);

    auto actors = localPlayer->level->getRuntimeActorList();
    for(auto& entity : actors) {
        if(!TargetUtil::isTargetValid(entity, false))
            continue;
        if(!selfESP && entity == localPlayer)
            continue;

        auto shape = entity->getAABBShapeComponent();
        auto renderPos = entity->getRenderPositionComponent();
        if(!shape || !renderPos)
            continue;

        Vec3<float> pos = renderPos->mPosition;
        float baseY = pos.y - shape->mHeight * 0.55f;
        float forwardOffset = shape->mWidth * 0.6f;

        float shaftL = 0.25f;
        float shaftH = 0.08f;
        float ballW = 0.08f;
        float ballH = 0.08f;
        float ballOffsetX = 0.12f;
        float ballOffsetY = -0.02f;
        float ballOffsetZ = 0.05f;

        AABB shaft;
        shaft.lower = Vec3<float>(pos.x - shaftH, baseY + ballH, pos.z + forwardOffset);
        shaft.upper =
            Vec3<float>(pos.x + shaftH, baseY + ballH + shaftH, pos.z + forwardOffset + shaftL);

        AABB ballL;
        ballL.lower = Vec3<float>(pos.x - ballW - ballOffsetX, baseY + ballOffsetY,
                                  pos.z + forwardOffset - ballOffsetZ);
        ballL.upper = Vec3<float>(pos.x - ballOffsetX, baseY + ballH + ballOffsetY,
                                  pos.z + forwardOffset - ballOffsetZ + ballW);

        AABB ballR;
        ballR.lower = Vec3<float>(pos.x + ballOffsetX, baseY + ballOffsetY,
                                  pos.z + forwardOffset - ballOffsetZ);
        ballR.upper = Vec3<float>(pos.x + ballW + ballOffsetX, baseY + ballH + ballOffsetY,
                                  pos.z + forwardOffset - ballOffsetZ + ballW);

        DrawUtil::drawBox3dFilled(shaft, UIColor(r, g, b, alpha), UIColor(r, g, b, 255));
        DrawUtil::drawBox3dFilled(ballL, UIColor(r, g, b, alpha), UIColor(r, g, b, 255));
        DrawUtil::drawBox3dFilled(ballR, UIColor(r, g, b, alpha), UIColor(r, g, b, 255));
    }
}
