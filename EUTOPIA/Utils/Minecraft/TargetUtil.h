#pragma once
#include <Maths.h>
// Forward declarations to avoid unnecessary heavy includes
class Actor;

class TargetUtil {
   public:
    static bool isTargetValid(Actor *target, bool isMob, float rangeCheck = 999999.f);
    static bool sortByDist(Actor *a1, Actor *a2);
    static AABB makeAABB(float width, float height, const Vec3<float> &lowerPos);
};
