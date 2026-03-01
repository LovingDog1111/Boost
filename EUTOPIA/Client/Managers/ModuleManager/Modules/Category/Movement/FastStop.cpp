#include "FastStop.h"



FastStop::FastStop() : Module("FastStop", "Instantly stop on key release", Category::MOVEMENT) {
}

void FastStop::onNormalTick(LocalPlayer* localPlayer) {
    static bool wasUsingMoveKeys = false;


    bool isUsingMoveKeys =
        GI::isKeyDown('W') || GI::isKeyDown('A') || GI::isKeyDown('S') || GI::isKeyDown('D');

    if(wasUsingMoveKeys && !isUsingMoveKeys) {
        localPlayer->getStateVectorComponent()->mVelocity.x = 0;
        localPlayer->getStateVectorComponent()->mVelocity.z = 0;
    }

    wasUsingMoveKeys = isUsingMoveKeys;
}
