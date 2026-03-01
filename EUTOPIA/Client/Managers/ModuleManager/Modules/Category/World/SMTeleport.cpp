#include "SMTeleport.h"

SMTeleport::SMTeleport() : Module("SMTeleport", "Smoothly teleports to a target", Category::WORLD) {
    registerSetting(new SliderSetting<float>("Zoomy", "Movement speed", &speed, 1.5f, 0.1f, 3.f));
    registerSetting(new SliderSetting<float>("Threshold", "Arrival distance", &arrivalThreshold,
                                             1.0f, 0.1f, 5.0f));
}

void SMTeleport::onEnable() {
    auto player = Game.getLocalPlayer();
    if(player)
        player->displayClientMessage("Use .setPos <x> <y> <z> to set a target");
    if(auto ci = GI::getClientInstance()) {
        if(auto mc = ci->minecraftSim) {
            prevTimer = mc->getSimTimer();
            mc->setSimTimer(static_cast<float>(timerValue));
            float s = static_cast<float>(timerValue) / 20.f;
            mc->setSimSpeed(s);
        }
    }
}

void SMTeleport::onDisable() {
    if(auto ci = GI::getClientInstance()) {
        if(auto mc = ci->minecraftSim) {
            mc->setSimTimer(prevTimer);
            mc->setSimSpeed(prevTimer / 20.f);
        }
    }
    auto player = Game.getLocalPlayer();
    if(player)
        resetAABB(player);
}

void SMTeleport::resetAABB(LocalPlayer* player) {
    AABB a = player->getAABB(true);
    a.upper.y = a.lower.y + 1.8f;
    player->setAABB(a);
}

void SMTeleport::onNormalTick(LocalPlayer* localPlayer) {
    auto player = Game.getLocalPlayer();
    if(!player)
        return;
    if(!hasTarget) {
        player->displayClientMessage("[SMTeleport] No target set");
        setEnabled(false);
        return;
    }
    Vec3<float> cur = player->getPos();
    float dX = targetPos.x - cur.x;
    float dY = targetPos.y - cur.y;
    float dZ = targetPos.z - cur.z;
    float dist = std::sqrt(dX * dX + dY * dY + dZ * dZ);
    if(dist <= arrivalThreshold) {
        resetAABB(player);
        player->displayClientMessage("[SMTeleport] Reached target");
        setEnabled(false);
        return;
    }
    float step = speed;
    if(step > dist)
        step = dist;
    float s = step / dist;
    Vec3<float> delta(dX * s, dY * s, dZ * s);
    player->lerpMotion(delta);
    AABB a = player->getAABB(true);
    a.lower = a.lower.add(delta);
    a.upper = a.upper.add(delta);
    player->setAABB(a);
}

void SMTeleport::setTargetPos(Vec3<float> pos) {
    targetPos = pos;
    hasTarget = true;
    auto player = Game.getLocalPlayer();
    if(player) {
        std::ostringstream ss;
        ss << "Target set: X" << pos.x << " Y" << pos.y << " Z" << pos.z;
        player->displayClientMessage(ss.str());
    }
}
