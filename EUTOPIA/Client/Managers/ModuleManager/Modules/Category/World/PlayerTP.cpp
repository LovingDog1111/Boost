#include "PlayerTP.h"

PlayerTP::PlayerTP() : Module("PlayerTP", "teleport to a chosen player.", Category::PLAYER) {
    registerSetting(new SliderSetting<float>("Speed", "Movement speed", &speed, 1.5f, 0.1f, 5.0f));
    registerSetting(
        new SliderSetting<float>("Threshold", "Stop distance", &threshold, 1.0f, 0.1f, 5.0f));
}

void PlayerTP::resetAABB(LocalPlayer* p) {
    AABB a = p->getAABB(true);
    a.upper.y = a.lower.y + 1.8f;
    p->setAABB(a);
}

void PlayerTP::setDefaultTimer() {
    if(auto ci = GI::getClientInstance(); ci && ci->minecraftSim) {
        ci->minecraftSim->setSimTimer(20.f);
    }
}

void PlayerTP::setTimerFast() {
    if(auto ci = GI::getClientInstance(); ci && ci->minecraftSim) {
        ci->minecraftSim->setSimTimer(200.f);
    }
}

void PlayerTP::onEnable() {
    if(!target) {
        Game.getLocalPlayer()->displayClientMessage("[PlayerTP] No target selected");
        this->setEnabled(false);
        return;
    }
    setTimerFast();
}

void PlayerTP::onNormalTick(LocalPlayer* player) {
    if(!player || !target)
        return;

    bool validTarget = false;
    std::vector<Actor*> actors = ActorUtils::getActorList(false);
    for(auto* entity : actors) {
        if(entity == target) {
            validTarget = true;
            break;
        }
    }

    if(!validTarget) {
        player->displayClientMessage("[PlayerTP] Target left the game or died");
        setDefaultTimer();
        target = nullptr;
        this->setEnabled(false);
        return;
    }


    Vec3<float> cur = player->getPos();
    Vec3<float> goal = target->getPos();

    Vec3<float> diff = goal.sub(cur);
    float dist = diff.magnitude();

    if(dist <= threshold) {
        resetAABB(player);
        setDefaultTimer();
        player->displayClientMessage("[PlayerTP] Reached target");
        this->setEnabled(false);
        return;
    }

    float step = std::min(speed, dist);
    float len = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
    Vec3<float> move((diff.x / len) * step, (diff.y / len) * step, (diff.z / len) * step);


    player->lerpMotion(move);

    AABB a = player->getAABB(true);
    a.lower = a.lower.add(move);
    a.upper = a.upper.add(move);
    player->setAABB(a);
}

void PlayerTP::onDisable() {
    setDefaultTimer();
    if(auto p = Game.getLocalPlayer())
        resetAABB(p);
}
