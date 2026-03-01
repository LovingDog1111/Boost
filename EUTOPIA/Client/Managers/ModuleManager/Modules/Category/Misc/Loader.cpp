#include "Loader.h"



Loader::Loader() : Module("Loader", "Loads entities every tick", Category::MISC) {}

void Loader::onNormalTick(LocalPlayer* lp) {
    if(!lp)
        return;
    for(Actor* actor : ActorUtils::getActorList(true)) {
        if(!actor)
            continue;
        actor->getPos(); // idk just to make sure
    }
}

void Loader::onEnable() {
    GI::DisplayClientMessage("[Loader] Loading entities..");
}

void Loader::onDisable() {
    GI::DisplayClientMessage("[Loader] Disabled");
}
