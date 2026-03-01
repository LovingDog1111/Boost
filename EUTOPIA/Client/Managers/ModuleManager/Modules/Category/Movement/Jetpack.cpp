#include "Jetpack.h"

Jetpack::Jetpack() : Module("Jetpack", "It JetPack :D", Category::MOVEMENT) {
    registerSetting(new SliderSetting<float>("Speed", "NULL", &speed, 1.f, 0.2f, 3.f));
}

void Jetpack::onNormalTick(LocalPlayer* localPlayer) {
    auto rotationComponent = localPlayer->getActorRotationComponent();
    if(!rotationComponent)
        return;

    float calcYaw = (rotationComponent->mYaw + 90.f) * (PI / 180.f);
    float calcPitch = (rotationComponent->mOldPitch) * -(PI / 180.f);

    Vec3<float> moveVec;
    moveVec.x = cos(calcYaw) * cos(calcPitch) * speed;
    moveVec.y = sin(calcPitch) * speed;
    moveVec.z = sin(calcYaw) * cos(calcPitch) * speed;

    localPlayer->lerpMotion(moveVec);
}