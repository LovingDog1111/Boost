#include "Jesus.h"

Jesus::Jesus() : Module("Jesus", "Walk over water, like Jesus.", Category::MOVEMENT) {}
Jesus::~Jesus() {}
void Jesus::onNormalTick(LocalPlayer* gm) {
    if(gm->getMoveInputComponent()->mIsSneakDown)
        return;

    if(GI::getClientInstance()
           ->getRegion()
           ->getBlock(gm->getPos().toInt())
           ->blockLegacy->blockName.find("water") != std::string::npos) {
        gm->getStateVectorComponent()->mVelocity.y = 0.06f;
        wasInWater = true;
    } else if(gm->isInWater() || gm->isInLava()) {
        auto player = GI::getLocalPlayer();
        gm->getStateVectorComponent()->mVelocity.y = 0.1f;
        wasInWater = true;
    } else {
        if(wasInWater) {
            wasInWater = false;
            gm->getStateVectorComponent()->mVelocity.x *= 1.2f;
            gm->getStateVectorComponent()->mVelocity.x *= 1.2f;
        }
    }
}
