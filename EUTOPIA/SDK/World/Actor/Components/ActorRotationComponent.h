//
// Created by vastrakai on 6/30/2024.
//

#pragma once
#include "../../../../Utils/Maths.h"

struct ActorRotationComponent {
public:
    float mPitch;
    float mYaw;
    float mOldPitch;
    float mOldYaw;
    Vec2<float> presentRot;
    Vec2<float> prevRot;
};

static_assert(sizeof(ActorRotationComponent) == 0x20);