#include "AirJump.h"


AirJump::AirJump()
    : Module("AirJump", "Allow you to jump even you're not on the ground", Category::MOVEMENT) {
    registerSetting(new BoolSetting("Legacy", "Auto jump while holding space", &legacy, false));
}
void AirJump::onLevelTick(Level* level) {
    auto lp = GI::getLocalPlayer();
    if(!lp)
        return;
    bool spaceDown = GetAsyncKeyState(VK_SPACE) & 0x8000;
    if(legacy) {
        if(spaceDown) {
            lp->setOnGround(true);
            lp->jumpFromGround();
        }
    } else {
        if(spaceDown && !prevSpace) {
            lp->setOnGround(true);
            lp->jumpFromGround();
        }
    }
    prevSpace = spaceDown;
}
