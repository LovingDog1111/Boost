#include "AutoMove.h"

#include <cmath>
#include "..\Client\Managers\ModuleManager\Modules\Category\Client\Audio.h"


AutoMove::AutoMove() : Module("AutoMove", "Move automatically", Category::MOVEMENT) {
    speed = 0.25f;
}

void AutoMove::onEnable() {
}

void AutoMove::onDisable() {}

void AutoMove::onNormalTick(LocalPlayer* player) {
    if(!player)
        return;

    MoveInputComponent* move = player->getMoveInputComponent();
    if(!move)
        return;

    Vec3<float> pos = player->getPos();
    Vec3<float> targetPos = pos.add2(0.f, 0.f, 5.f);  

    Vec3<float> diff = targetPos.sub(pos);
    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

    bool walkBackward = dist < 3.0f;

    float yawRad = 0.f; 
    if(walkBackward)
        yawRad += 3.14159265f;  

    move->mMoveVector.x = std::cos(yawRad);
    move->mMoveVector.y = std::sin(yawRad);
    move->mForward = !walkBackward && dist > 0.5f;
    move->mBackward = walkBackward && dist > 0.5f;
}
