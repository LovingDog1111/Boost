#include "Twerk.h"

Twerk::Twerk() : Module("Twerk", "Twerk it lol", Category::MISC) {}

void Twerk::onNormalTick(LocalPlayer* localPlayer) {
    sneakState = !sneakState; 
    localPlayer->getMoveInputComponent()->setSneaking(sneakState);
}

void Twerk::onDisable() {
    GI::getLocalPlayer()->getMoveInputComponent()->setSneaking(false);
}
