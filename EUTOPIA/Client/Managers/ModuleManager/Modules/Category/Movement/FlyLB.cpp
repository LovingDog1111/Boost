#include "FlyLB.h"

#include <Windows.h>

#include <cmath>

bool crouch2 = false;
constexpr float CONST_PI = 3.141592653589793f;

FlyLB::FlyLB() : Module("FlyLB", "Fly like in creative mode", Category::MOVEMENT) {
    registerSetting(
        new SliderSetting<float>("Horizontal Speed", "Horizontal speed", &hSpeed, 1.f, 0.2f, 11.f));
    registerSetting(
        new SliderSetting<float>("Vertical Speed", "Vertical speed", &vSpeed, 0.5f, 0.2f, 5.f));
    registerSetting(new SliderSetting<float>("Glide Value", "Value that how much u want to go down",
                                             &Glide, -0.02, -0.75, 0.f));
    registerSetting(new BoolSetting("Sm Border", "Avoid getting damage by the lifeboat SM borer",
                                    &smborder, false));
    registerSetting(new BoolSetting("Crouch", "Sneak while flying", &crouch2, false));
    registerSetting(
        new BoolSetting("Bypass", "Avoid getting damage by the lifeboat SM borer", &bypass, false));
    registerSetting(new BoolSetting("Dynamic", "Less strict", &dynamic, false));
}

void FlyLB::onNormalTick(LocalPlayer* localPlayer) {
    auto state = localPlayer->getStateVectorComponent();
    if(state == nullptr)
        return;

    currentSpeed = hSpeed;
    Vec3<float>& velocity = state->mVelocity;
    velocity = Vec3<float>(0.f, 0.f, 0.f);
    velocity.y += Glide;

    if(crouch2) {
        Game.getLocalPlayer()->getMoveInputComponent()->setSneaking(true);
    }

    if(Game.canUseMoveKeys()) {
        float yaw = localPlayer->getRotation().x;

        bool isForward = Game.isKeyDown('W');
        bool isLeft = Game.isKeyDown('A');
        bool isBackward = Game.isKeyDown('S');
        bool isRight = Game.isKeyDown('D');
        bool isUp = Game.isKeyDown(VK_SPACE);
        bool isDown = Game.isKeyDown(VK_SHIFT);

        Vec2<int> moveValue;
        if(isRight)
            moveValue.x += 1;
        if(isLeft)
            moveValue.x -= 1;
        if(isForward)
            moveValue.y += 1;
        if(isBackward)
            moveValue.y -= 1;

        if(dynamic && isUp) {
            currentSpeed = 1.0f;
            vSpeed = 1.0f;
        }

        if(isUp)
            velocity.y += vSpeed;
        if(isDown)
            velocity.y -= vSpeed;

        float angleRad = std::atan2(moveValue.x, moveValue.y);
        float angleDeg = angleRad * (180.f / CONST_PI);
        yaw += angleDeg;

        Vec3<float> moveVec = Vec3<float>(0.f, velocity.y, 0.f);
        if(moveValue.x != 0 || moveValue.y != 0) {
            float calcYaw = (yaw + 90.f) * (CONST_PI / 180.f);
            moveVec.x = cos(calcYaw) * currentSpeed;
            moveVec.z = sin(calcYaw) * currentSpeed;
        }

        if(smborder) {
            Vec3<float> pos = localPlayer->getPos();
            float newX = pos.x + moveVec.x;
            float newY = pos.y + moveVec.y;
            float newZ = pos.z + moveVec.z;


            if(newX < 5.f)
                newX = 5.f;
            if(newX > 1199.f)
                newX = 1199.f;

            if(newZ < 5.f)
                newZ = 5.f;
            if(newZ > 1199.f)
                newZ = 1199.f;


            moveVec.x = newX - localPlayer->getPos().x;
            moveVec.y = newY - localPlayer->getPos().y;
            moveVec.z = newZ - localPlayer->getPos().z;

        }

        localPlayer->lerpMotion(moveVec);
    }
}

void FlyLB::onDisable() {
    if(crouch2) {
        Game.getLocalPlayer()->getMoveInputComponent()->setSneaking(false);
    }
}

void FlyLB::onSendPacket(Packet* packet) {
    if((packet->getName() == "PlayerAuthInputPacket" || packet->getName() == "MovePlayerPacket")) {
        auto* paip = (PlayerAuthInputPacket*)packet;
        auto* mpp = (MovePlayerPacket*)packet;
        static float offsetY = 0.2f;
        LocalPlayer* localPlayer = Game.getLocalPlayer();
        if(localPlayer == nullptr) {
            offsetY = 0.2f;
            return;
        }

        if(paip) {
            if(bypass) {
                if(packet->getId() == PacketID::PlayerAuthInput) {
                    PlayerAuthInputPacket* paip = static_cast<PlayerAuthInputPacket*>(packet);
                    paip->mPos.y -= offsetY;
                    offsetY += 0.012f;
                    if(offsetY >= 0.44f) {
                        offsetY = 0.2f;
                    }
                }
            }
        }
    }
}
