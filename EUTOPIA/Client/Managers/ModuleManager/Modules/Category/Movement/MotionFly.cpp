#include "MotionFly.h"

#include <Windows.h>

#include <cmath>

#include "../../../ModuleManager.h"

constexpr float CONST_PI = 3.141592653589793f;

MotionFly::MotionFly() : Module("MotionFly", "Motion fly like creative", Category::MOVEMENT) {
    registerSetting(new SliderSetting<float>("Speed", "", &hSpeed, 1.f, 0.2f, 15.f));
    registerSetting(new SliderSetting<float>("Vertical Speed", "", &vSpeed, 0.5f, 0.2f, 3.f));
    registerSetting(new SliderSetting<float>("Glide Rate", "", &Glide, -0.02f, -0.6f, 0.f));
    registerSetting(new BoolSetting("Timer Boost", "", &timerBoost, false));
    registerSetting(
        new SliderSetting<float>("Timer Boost Value", "", &timerBoostValue, 1.f, 0.1f, 5.f));
    registerSetting(new BoolSetting("Apply Glide Flags", "", &applyGlideFlags, false));
}

void MotionFly::onEnable() {
    GI::DisplayClientMessage("%s[MotionFly] Enabled", MCTF::GREEN);
}

void MotionFly::onNormalTick(LocalPlayer* localPlayer) {
    auto state = localPlayer->getStateVectorComponent();
    if(!state)
        return;

    Vec3<float>& velocity = state->mVelocity;
    velocity = Vec3<float>(0.f, 0.f, 0.f);

    if(GI::canUseMoveKeys()) {
        float yaw = localPlayer->getRotation().x;
        bool isForward = GI::isKeyDown('W');
        bool isLeft = GI::isKeyDown('A');
        bool isBackward = GI::isKeyDown('S');
        bool isRight = GI::isKeyDown('D');
        bool isJumping = GI::isKeyDown(VK_SPACE);
        bool isSneaking = GI::isKeyDown(VK_SHIFT);

        Vec2<int> moveValue{};
        if(isRight)
            moveValue.x += 1;
        if(isLeft)
            moveValue.x -= 1;
        if(isForward)
            moveValue.y += 1;
        if(isBackward)
            moveValue.y -= 1;

        if(isJumping)
            velocity.y += hSpeed / 10;
        else if(isSneaking)
            velocity.y -= hSpeed / 10;

        float angleRad = std::atan2(moveValue.x, moveValue.y);
        float angleDeg = angleRad * (180.f / CONST_PI);
        yaw += angleDeg;

        Vec3<float> moveVec(0.f, velocity.y, 0.f);
        if(moveValue.x != 0 || moveValue.y != 0) {
            float calcYaw = (yaw + 90.f) * (CONST_PI / 180.f);
            moveVec.x = cos(calcYaw) * hSpeed / 10;
            moveVec.z = sin(calcYaw) * hSpeed / 10;
        }
        localPlayer->lerpMotion(moveVec);
    }

    if(timerBoost)
        GI::getClientInstance()->minecraftSim->setSimTimer(timerBoostValue);
    else
        GI::getClientInstance()->minecraftSim->setSimTimer(20);
}

void MotionFly::onSendPacket(Packet* packet) {
    if(!applyGlideFlags)
        return;
    if(packet->getId() != PacketID::PlayerAuthInput)
        return;
    auto p = static_cast<PlayerAuthInputPacket*>(packet);
    p->mInputData |= AuthInputAction::START_GLIDING;
    p->mInputData &= ~AuthInputAction::STOP_GLIDING;
}

void MotionFly::onDisable() {
    GI::DisplayClientMessage("%s[MotionFly] Disabled", MCTF::RED);
    GI::getClientInstance()->minecraftSim->setSimTimer(20);
}
