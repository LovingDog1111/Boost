#include "Fly.h"

#include <cmath>
#include <Windows.h>

bool crouch = false;

constexpr float CONST_PI = 3.141592653589793f;

Fly::Fly()
    : Module("Fly", "Fly like in creative mode",
             Category::MOVEMENT) {
    registerSetting(new SliderSetting<float>(
        "Horizontal Speed", "horizontal flying speed", &hSpeed, 1.f, 0.2f, 15.f));
    registerSetting(new SliderSetting<float>(
        "Vertical Speed", "vertical flying speed", &vSpeed, 0.5f, 0.2f, 3.f));
    registerSetting(new SliderSetting<float>(
        "Glide Rate", "Adjusts how much you descend while gliding", &Glide, -0.02, -0.6, 0.f));
    registerSetting(new BoolSetting("Dynamic", "Less strict", &dynamic, false));
  

}

#include "../../../ModuleManager.h"


void Fly::onEnable() {
    GI::DisplayClientMessage("%s[Fly] Enabled", MCTF::GREEN);
}

void Fly::onNormalTick(LocalPlayer* localPlayer) {
    auto state = localPlayer->getStateVectorComponent();
    if(state == nullptr) {
        return;
    }


    // Update current speed from slider each tick
    currentSpeed = hSpeed;

    Vec3<float>& velocity = state->mVelocity;
    velocity = Vec3<float>(0.f, 0.f, 0.f);
    velocity.y += Glide;

    if(GI::canUseMoveKeys()) {
        float yaw = localPlayer->getRotation().x;

        bool isForward = GI::isKeyDown('W');
        bool isLeft = GI::isKeyDown('A');
        bool isBackward = GI::isKeyDown('S');
        bool isRight = GI::isKeyDown('D');
        bool isUp = GI::isKeyDown(VK_SPACE);
        bool isDown = GI::isKeyDown(VK_SHIFT);

        Vec2<int> moveValue;
        if(isRight)
            moveValue.x += 1;
        if(isLeft)
            moveValue.x += -1;
        if(isForward)
            moveValue.y += 1;
        if(isBackward)
            moveValue.y += -1;

        if(dynamic && isUp) {
            currentSpeed = 1.0f;
            vSpeed = 1.0f;
        }

        if(isUp)
            velocity.y += vSpeed;
        if(isDown)
            velocity.y -= vSpeed;

        float angleRad = (float)std::atan2(moveValue.x, moveValue.y);
        float angleDeg = angleRad * (180.f / CONST_PI);
        yaw += angleDeg;
        Vec3<float> moveVec = Vec3<float>(0.f, velocity.y, 0.f);
        if((moveValue.x != 0) || (moveValue.y != 0)) {
            float calcYaw = (yaw + 90.f) * (CONST_PI / 180.f);
            moveVec.x = cos(calcYaw) * currentSpeed;
            moveVec.z = sin(calcYaw) * currentSpeed;
        }
        localPlayer->lerpMotion(moveVec);
    }
}

void Fly::onSendPacket(Packet* packet) {}

void Fly::onDisable() {
    auto player = GI::getLocalPlayer();
    
    GI::DisplayClientMessage("%s[Fly] Disabled", MCTF::RED);
}