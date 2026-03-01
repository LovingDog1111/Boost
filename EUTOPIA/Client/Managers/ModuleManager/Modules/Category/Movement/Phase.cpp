#include "Phase.h"

Phase::Phase() : Module("Phase", "Allow you walk through blocks/walls", Category::MOVEMENT) {}

void Phase::onDisable() {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(localPlayer == nullptr)
        return;

    AABB currentAABB = localPlayer->getAABB(true);
    if(currentAABB.upper.y == currentAABB.lower.y) {
        currentAABB.upper.y = currentAABB.lower.y + 1.8f;
        localPlayer->setAABB(currentAABB);
    }
}

void Phase::onLevelTick(Level* level) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(localPlayer == nullptr)
        return;

    AABB currentAABB = localPlayer->getAABB(true);
    currentAABB.upper.y = currentAABB.lower.y;
    localPlayer->setAABB(currentAABB);
}