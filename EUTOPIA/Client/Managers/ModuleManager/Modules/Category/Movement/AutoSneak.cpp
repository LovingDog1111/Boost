#include "AutoSneak.h"

AutoSneak::AutoSneak() : Module("AutoSneak", "Sneak Automatically", Category::MOVEMENT) {}

void AutoSneak::onNormalTick(LocalPlayer* localPlayer) {
    GI::getLocalPlayer()->getMoveInputComponent()->setSneaking(true);
}

void AutoSneak::onDisable() {
    GI::getLocalPlayer()->getMoveInputComponent()->setSneaking(false);
}