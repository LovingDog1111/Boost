#include "Freecam.h"

#include <Windows.h>

#include <cmath>

bool crouch23233232 = false;
float hspeed;
float vspeed;
float hSpeed = 1.0f;
float vSpeed = 0.5f;
float Glide = -0.05f;
bool dynamic = false;
float currentSpeed;
constexpr float CONST_PI = 3.141592653589793f;
bool tpondisable;
Vec3<float> freecamPos;

Freecam::Freecam() : Module("Freecam", "Fly without moving the player", Category::MOVEMENT) {

    registerSetting(new SliderSetting<float>("Horizontal Speed", "horizontal flying speed", &hSpeed,
                                             1.f, 0.2f, 15.f));
    registerSetting(new SliderSetting<float>("Vertical Speed", "vertical flying speed", &vSpeed,
                                             0.5f, 0.2f, 3.f));
    registerSetting(new SliderSetting<float>(
        "Glide Rate", "Adjusts how much you descend while gliding", &Glide, -0.02, -0.6, 0.f));
    registerSetting(new BoolSetting("Dynamic", "Less strict", &dynamic, false));
}

#include "../../../ModuleManager.h"

Freecam::~Freecam() {}

void Freecam::onEnable() {
    keyPressed = false;
    auto player = GI::getLocalPlayer();
    if(!player)
        return;
    GI::DisplayClientMessage("%s[Freecam] Enabled", MCTF::GREEN);
    Vec3<float> pos = player->getPos();
    freecamPos = pos;
    Vec3<float> tinySize(0.1f, 0.1f, 0.1f);
    Vec3<float> min(pos.x - tinySize.x / 2, pos.y, pos.z - tinySize.z / 2);
    Vec3<float> max(pos.x + tinySize.x / 2, pos.y + tinySize.y, pos.z + tinySize.z / 2);
    AABB aabb(min, max);
    player->setAABB(aabb);
}

void Freecam::onMCRender(MinecraftUIRenderContext* rcx) {
    auto localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;
    auto state = localPlayer->getStateVectorComponent();
    if(!state)
        return;
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
        Vec3<float> moveVec(0.f, velocity.y, 0.f);
        if(moveValue.x != 0 || moveValue.y != 0) {
            float calcYaw = (yaw + 90.f) * (CONST_PI / 180.f);
            moveVec.x = cos(calcYaw) * currentSpeed;
            moveVec.z = sin(calcYaw) * currentSpeed;
        }
        Vec3<float> newPos = localPlayer->getPos() + moveVec;
        freecamPos = newPos;
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

void Freecam::onDisable() {
    auto player = GI::getLocalPlayer();
    if(tpondisable) {
        keyPressed = true;  // indicate that its disabling (loo lazy to recompile)
    }
    if(!player)
        return;
    GI::DisplayClientMessage("%s[Freecam] Disabled", MCTF::RED);
    Vec3<float> pos = player->getPos();
    Vec3<float> normalSize(0.6f, 1.8f, 0.6f);
    Vec3<float> min(pos.x - normalSize.x / 2, pos.y, pos.z - normalSize.z / 2);
    Vec3<float> max(pos.x + normalSize.x / 2, pos.y + normalSize.y, pos.z + normalSize.z / 2);
    AABB aabb(min, max);
    player->setAABB(aabb);
}
