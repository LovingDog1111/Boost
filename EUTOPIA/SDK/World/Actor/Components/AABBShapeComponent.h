//
// Created by vastrakai on 6/28/2024.
//

#pragma once


#include "../../../../Utils/Maths.h"

struct AABBShapeComponent {
    Vec3<float> mMin;
    Vec3<float> mMax;
    float mWidth;
    float mHeight;

    AABB getAABB() const {
        return AABB(mMin, mMax, true);
    }
};

static_assert(sizeof(AABBShapeComponent) == 0x20);