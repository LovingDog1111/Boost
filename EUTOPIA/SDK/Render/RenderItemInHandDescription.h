#pragma once
#include <MemoryUtil.h>
#include <Maths.h>
class RenderItemInHandDescription {
   public:
    CLASS_MEMBER(Vec3<float>, mGlintColor, 0x8C);
    CLASS_MEMBER(float, mGlintAlpha, 0x9C);
    CLASS_MEMBER(Vec3<float>, mOverlayColor, 0xAC);
    CLASS_MEMBER(float, mOverlayAlpha, 0xBC);
    CLASS_MEMBER(Vec3<float>, mChangeColor, 0xCC);
    CLASS_MEMBER(float, mChangeAlpha, 0xDC);
    CLASS_MEMBER(Vec3<float>, mMultiplicativeTintColor, 0xEC);
    CLASS_MEMBER(float, mMultiplicativeTintColorAlpha, 0xFC);
};