#include "EnemyTP.h"

#include "..\Utils\Minecraft\TargetUtil.h"

const int MIN_X = 60;
const int MAX_X = 1090;
const int MIN_Z = 60;
const int MAX_Z = 1090;

bool followplayer = false;
float speed = 1.5f;
float arrivalThreshold = 1.0f;
float prevTimer = 20.f;

int EnemyTP::tickCounter = 0;
bool EnemyTP::works = false;
bool EnemyTP::tpdone = false;
float EnemyTP::closestDistance = 0.f;
Vec3<float> EnemyTP::goal = Vec3<float>(0, 0, 0);
std::vector<Actor*> EnemyTP::targetListTP;

EnemyTP::EnemyTP()
    : Module("EnemyTP", "Teleport to the nearest player with a certain speed.", Category::PLAYER) {
    registerSetting(new SliderSetting<int>("Range", "NULL", &range, 60, 3, 500));
    registerSetting(new SliderSetting<float>("Speed", "Teleport speed", &speed, 1.5f, 0.1f, 3.f));
    registerSetting(new SliderSetting<float>("Threshold", "Arrival distance", &arrivalThreshold,
                                             1.0f, 0.1f, 5.0f));
}

bool EnemyTP::sortByDist(Actor* a1, Actor* a2) {
    Vec3<float> lpPos = Game.getLocalPlayer()->getPos();
    return a1->getPos().dist(lpPos) < a2->getPos().dist(lpPos);
}

void EnemyTP::resetAABB(LocalPlayer* player) {
    AABB a = player->getAABB(true);
    a.upper.y = a.lower.y + 1.8f;
    player->setAABB(a);
}

void EnemyTP::setDefaultTimer() {
    if(auto ci = GI::getClientInstance()) {
        if(auto mc = ci->minecraftSim) {
            mc->setSimTimer(prevTimer);
            mc->setSimSpeed(prevTimer / 20.f);
        }
    }
}

void EnemyTP::setTimerFast() {
    if(auto ci = GI::getClientInstance()) {
        if(auto mc = ci->minecraftSim) {
            mc->setSimTimer(1209.f);
            mc->setSimSpeed(1209.f / 20.f);
        }
    }
}

void EnemyTP::onEnable() {
    LocalPlayer* localPlayer = Game.getLocalPlayer();
    if(!localPlayer)
        return;

    Level* level = localPlayer->level;
    if(!level)
        return;

    targetListTP.clear();

    prevTimer = 20.f; 
    setTimerFast();

    works = false;
    tpdone = false;
    closestDistance = std::numeric_limits<float>::max();
    goal = Vec3<float>(0, 0, 0);

    std::vector<Actor*> actors = ActorUtils::getActorList(false);
    for(auto* entity : actors) {
        if(TargetUtil::isTargetValid(entity, false)) {
            float dist = localPlayer->getPos().dist(entity->getPos());
            if(dist <= range) {
                targetListTP.push_back(entity);
                if(dist < closestDistance) {
                    closestDistance = dist;
                    goal = entity->getPos();
                }
            }
        }
    }

    works = closestDistance < std::numeric_limits<float>::max();
    if(!works)
        this->setEnabled(false);
}

void EnemyTP::onNormalTick(LocalPlayer* player) {
    if(!this->isEnabled())
        return;
    if(!player) {
        this->setEnabled(false);
        return;
    }
    if(!works) {
        this->setEnabled(false);
        return;
    }
    if(tpdone) {
        this->setEnabled(false);
        return;
    }

    Vec3<float> cur = player->getPos();
    Vec3<float> delta(goal.x - cur.x, goal.y - cur.y, goal.z - cur.z);
    float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);

    if(dist <= arrivalThreshold) {
        resetAABB(player);
        tpdone = true;
        setDefaultTimer();
        player->displayClientMessage("[EnemyTP] Reached target");
        this->setEnabled(false);
        return;
    }

    float step = std::min(speed, dist);
    float ratio = step / dist;
    Vec3<float> move(delta.x * ratio, delta.y * ratio, delta.z * ratio);

    player->lerpMotion(move);

    AABB a = player->getAABB(true);
    a.lower = a.lower.add(move);
    a.upper = a.upper.add(move);
    player->setAABB(a);
}

void EnemyTP::onDisable() {
    setDefaultTimer();
    LocalPlayer* player = Game.getLocalPlayer();
    if(player)
        resetAABB(player);
}
