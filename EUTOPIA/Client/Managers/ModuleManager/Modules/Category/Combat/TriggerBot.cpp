#include "TriggerBot.h"

int delay = 0;

TriggerBot::TriggerBot() : Module("TriggerBot", "Attacks any entity you look at", Category::COMBAT) {
    registerSetting(new SliderSetting<int>("Delay", "Attack delay (Tick)", &delay, 0, 0, 20));
}

Actor* getActorFromEntityId(EntityId entityId) {
    auto player = GI::getLocalPlayer();
    for(auto actor : player->level->getRuntimeActorList()) {
        if(actor->getEntityContext()->entity.rawId == entityId.rawId) {
            return actor;
        }
    }
    return nullptr;
}

void TriggerBot::onNormalTick(LocalPlayer* localPlayer) {
    auto player = GI::getLocalPlayer();
    if(!player)
        return;

    HitResult* hitResult = player->level->getHitResult();
    if(hitResult->type != HitResultType::ENTITY)
        return;

    Actor* hitActor = getActorFromEntityId(hitResult->entity.id);
    if(!hitActor)
        return;

    if(hitActor->getEntityContext()->entity.rawId == player->getEntityContext()->entity.rawId)
        return;

    static int tickDelay = 0;
    if(tickDelay > 0) {
        tickDelay--;
        return;
    }

    auto gameMode = player->getgamemode();
    if(!gameMode)
        return;

    gameMode->attack(hitActor);
    player->swing();

    tickDelay = delay;
}
