#include "AutoJump.h"

AutoJump::AutoJump() : Module("AutoJump", "Jump Automatically", Category::MOVEMENT) {}

void AutoJump::onNormalTick(LocalPlayer* localPlayer) {
    GI::getLocalPlayer()->getMoveInputComponent()->setJumping(true);
}

void AutoJump::onDisable() {
    GI::getLocalPlayer()->getMoveInputComponent()->setJumping(false);
}