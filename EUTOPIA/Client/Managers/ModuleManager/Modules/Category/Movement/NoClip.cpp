#include "NoClip.h"

#include <Windows.h>

#include <cmath>

constexpr float CONST_PI = 3.141592653589793f;

NoClip::NoClip() : Module("NoClip", "Fly through blocks/walls", Category::MOVEMENT) {
    registerSetting(new SliderSetting<float>("Horizontal Speed", "Horizontal movement speed",
                                             &hSpeed, 1.f, 0.2f, 11.f));
    registerSetting(new SliderSetting<float>("Vertical Speed", "Vertical movement speed", &vSpeed,
                                             0.5f, 0.2f, 5.f));
    registerSetting(new SliderSetting<float>("Glide Value", "Passive downward movement", &Glide,
                                             -0.02f, -0.75f, 0.f));
}

void NoClip::onEnable() {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;

    AABB currentAABB = localPlayer->getAABB(true);
    currentAABB.upper.y = currentAABB.lower.y;
    localPlayer->setAABB(currentAABB);
}

void NoClip::onDisable() {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;

    AABB currentAABB = localPlayer->getAABB(true);
    currentAABB.upper.y = currentAABB.lower.y + 1.8f;
    localPlayer->setAABB(currentAABB);
}

void NoClip::onNormalTick(LocalPlayer* localPlayer) {
    if(!localPlayer)
        return;
    auto state = localPlayer->getStateVectorComponent();
    if(!state)
        return;

    state->mVelocity = Vec3<float>(0.f, Glide, 0.f);

    if(GI::canUseMoveKeys()) {
        bool isForward = GI::isKeyDown('W');
        bool isLeft = GI::isKeyDown('A');
        bool isBackward = GI::isKeyDown('S');
        bool isRight = GI::isKeyDown('D');
        bool isUp = GI::isKeyDown(VK_SPACE);
        bool isDown = GI::isKeyDown(VK_SHIFT);

        Vec2<int> moveValue(0, 0);
        if(isRight)
            moveValue.x += 1;
        if(isLeft)
            moveValue.x -= 1;
        if(isForward)
            moveValue.y += 1;
        if(isBackward)
            moveValue.y -= 1;

        float yaw = localPlayer->getRotation().x;
        Vec3<float> moveVec(0.f, state->mVelocity.y, 0.f);

        if(moveValue.x != 0 || moveValue.y != 0) {
            float angleRad = std::atan2(moveValue.x, moveValue.y);
            float angleDeg = angleRad * (180.f / CONST_PI);
            yaw += angleDeg;
            float calcYaw = (yaw + 90.f) * (CONST_PI / 180.f);
            moveVec.x = cos(calcYaw) * hSpeed;
            moveVec.z = sin(calcYaw) * hSpeed;
        }

        if(isUp)
            moveVec.y += vSpeed;
        if(isDown)
            moveVec.y -= vSpeed;

        Vec3<float> newPos = localPlayer->getPos() + moveVec;

        AABB currentAABB = localPlayer->getAABB(true);
        currentAABB.lower.x += moveVec.x;
        currentAABB.lower.y += moveVec.y;
        currentAABB.lower.z += moveVec.z;

        currentAABB.upper.x += moveVec.x;
        currentAABB.upper.y += moveVec.y;
        currentAABB.upper.z += moveVec.z;

        localPlayer->setAABB(currentAABB);

        state->mVelocity = moveVec;
        localPlayer->lerpMotion(moveVec);

    }
}
