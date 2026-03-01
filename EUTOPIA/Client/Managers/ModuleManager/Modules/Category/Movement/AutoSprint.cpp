#include "AutoSprint.h"

AutoSprint::AutoSprint()
    : Module("AutoSprint", "Automatically sprint without holding the key", Category::MOVEMENT) {
    modeEnum =
        (EnumSetting*)registerSetting(new EnumSetting("Mode", "NULL", {"Legit", "Omni"}, &mode, 0));
}

std::string AutoSprint::getModeText() {
    return modeEnum->enumList[mode];
}

void AutoSprint::onNormalTick(LocalPlayer* localPlayer) {
    if(!GI::canUseMoveKeys())
        return;

    bool isSprinting = localPlayer->getStatusFlag(ActorFlags::Sprinting);
    if(isSprinting)
        return;

    if(mode == 0) {
        if(GI::isKeyDown('W'))
            localPlayer->setSprinting(true);
    } else {
        if(GI::isKeyDown('W') || GI::isKeyDown('A') || GI::isKeyDown('S') || GI::isKeyDown('D'))
            localPlayer->setSprinting(true);
    }
}