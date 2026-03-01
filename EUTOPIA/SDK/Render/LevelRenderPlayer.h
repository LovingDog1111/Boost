#pragma once
#include "../../Utils/MemoryUtil.h"
#include "../../Utils/Maths.h"

class LevelRendererPlayer {
public:
CLASS_MEMBER(Vec3<float>, cameraPos, 0x664);
CLASS_MEMBER(Vec3<float>, origin, 0x670)
CLASS_MEMBER(Vec3<float>, mOrigin, 0x670)
CLASS_MEMBER(Vec3<float>, Origin, 0x670)

float getFovX() {
    return hat::member_at<float>(this, 0x1020);
}

float getFovY() {
    return hat::member_at<float>(this, 0x1034);
}
};