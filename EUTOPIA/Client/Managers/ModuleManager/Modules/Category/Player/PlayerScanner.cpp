#include "PlayerScanner.h"

#include "..\Utils\Minecraft\TargetUtil.h"

PlayerScanner::PlayerScanner()
    : Module("PlayerScanner", "Scans the entire map for players using smooth movement",
             Category::PLAYER) {}

void PlayerScanner::resetAABB(LocalPlayer* player) {
    AABB a = player->getAABB(true);
    a.upper.y = a.lower.y + 1.8f;
    player->setAABB(a);
}

void PlayerScanner::onEnable() {
    LocalPlayer* player = Game.getLocalPlayer();
    if(!player) {
        this->setEnabled(false);
        return;
    }
    results.clear();
    scanTarget = player->getPos();
    scanning = true;
    waiting = false;
}

void PlayerScanner::onNormalTick(LocalPlayer* player) {
    if(!player || !scanning) {
        this->setEnabled(false);
        return;
    }

    if(waiting) {
        auto now = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(now - waitStart).count();
        if(elapsed < waitSeconds)
            return;
        waiting = false;
    }

    Vec3<float> cur = player->getPos();
    Vec3<float> delta(scanTarget.x - cur.x, scanTarget.y - cur.y, scanTarget.z - cur.z);
    float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    if(dist > arrivalThreshold) {
        float step = std::min(speed, dist);
        float ratio = step / dist;
        Vec3<float> move(delta.x * ratio, delta.y * ratio, delta.z * ratio);
        player->lerpMotion(move);
        AABB a = player->getAABB(true);
        a.lower = a.lower.add(move);
        a.upper = a.upper.add(move);
        player->setAABB(a);
        return;
    }

    std::vector<Actor*> actors = ActorUtils::getActorList(false);
    for(auto* entity : actors) {
        if(TargetUtil::isTargetValid(entity, false)) {
            Vec3<float> p = entity->getPos();
            bool exists = false;
            for(auto& r : results) {
                if(r.name == entity->getNameTag() && r.pos.dist(p) < 1.f) {
                    exists = true;
                    break;
                }
            }
            if(!exists)
                results.push_back({std::string(entity->getNameTag()), p});


        }
    }

    if(scanTarget.x + stepSize <= mapMax)
        scanTarget.x += stepSize;
    else if(scanTarget.z + stepSize <= mapMax) {
        scanTarget.x = mapMin;
        scanTarget.z += stepSize;
    } else {
        scanning = false;
        for(auto& r : results) {
            std::ostringstream ss;
            ss << "[PlayerScanner] " << r.name << " X" << (int)r.pos.x << " Y" << (int)r.pos.y
               << " Z" << (int)r.pos.z;
            GI::DisplayClientMessage(ss.str().c_str());
        }
        this->setEnabled(false);
        return;
    }

    waiting = true;
    waitStart = std::chrono::steady_clock::now();
}
