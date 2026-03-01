#include "MobTP.h"

#include <cmath>
#include <limits>

#include "../../Utils/Minecraft/TargetUtil.h"

bool MobTP::tpCows = true;
bool MobTP::tpSheeps = true;
bool MobTP::tpCreepers = true;
bool MobTP::tpZombies = true;
bool MobTP::tpSkeletons = true;
bool MobTP::tpSpiders = true;
bool tpendermen;

std::vector<Actor*> MobTP::targetList;
Vec3<float> MobTP::goal = Vec3<float>(0, 0, 0);
bool MobTP::works = false;
bool MobTP::tpDone = false;
float MobTP::closestDistance = 0.f;
float MobTP::speed = 1.5f;
float MobTP::arrivalThreshold = 1.0f;
float MobTP::prevTimer = 20.f;

MobTP::MobTP() : Module("MobTP", "Smooth teleport to nearest selected mob", Category::WORLD) {
    registerSetting(new BoolSetting("Cows", "", &tpCows, true));
    registerSetting(new BoolSetting("Sheeps", "", &tpSheeps, true));
    registerSetting(new BoolSetting("Creepers", "", &tpCreepers, true));
    registerSetting(new BoolSetting("Zombies", "", &tpZombies, true));
    registerSetting(new BoolSetting("Skeletons", "", &tpSkeletons, true));
    registerSetting(new BoolSetting("Spiders", "", &tpSpiders, true));
    registerSetting(new BoolSetting("Endermen", "", &tpendermen, true));
}

bool MobTP::sortByDist(Actor* a1, Actor* a2) {
    Vec3<float> lpPos = Game.getLocalPlayer()->getPos();
    return a1->getPos().dist(lpPos) < a2->getPos().dist(lpPos);
}

void MobTP::resetAABB(LocalPlayer* player) {
    if(!player)
        return;
    AABB a = player->getAABB(true);
    a.upper.y = a.lower.y + 1.8f;
    player->setAABB(a);
}

void MobTP::setTimerFast() {
    if(auto ci = GI::getClientInstance()) {
        if(auto mc = ci->minecraftSim) {
            mc->setSimTimer(1209.f);
            mc->setSimSpeed(1209.f / 20.f);
        }
    }
}

void MobTP::setDefaultTimer() {
    if(auto ci = GI::getClientInstance()) {
        if(auto mc = ci->minecraftSim) {
            mc->setSimTimer(prevTimer);
            mc->setSimSpeed(prevTimer / 20.f);
        }
    }
}

void MobTP::onEnable() {
    LocalPlayer* player = Game.getLocalPlayer();
    if(!player)
        return;
    Level* level = player->level;
    if(!level)
        return;

    targetList.clear();
    tpDone = false;
    closestDistance = std::numeric_limits<float>::max();
    goal = Vec3<float>(0, 0, 0);
    prevTimer = 20.f;

    setTimerFast();

    for(Actor* mob : level->getRuntimeActorList()) {
        if(!mob)
            continue;
        ActorType type = ActorType::Undefined;
        if(auto typeComp = mob->getActorTypeComponent())
            type = static_cast<ActorType>(typeComp->id);

        bool valid = (tpCows && type == ActorType::Cow) || (tpSheeps && type == ActorType::Sheep) ||
                     (tpCreepers && type == ActorType::Creeper) ||
                     (tpZombies && type == ActorType::Zombie) ||
                     (tpSkeletons && type == ActorType::Skeleton) ||
                     (tpSpiders && type == ActorType::Spider) ||
                     (tpendermen && type == ActorType::EnderMan);

        if(!valid)
            continue;

        float dist = player->getPos().dist(mob->getPos());
        targetList.push_back(mob);

        if(dist < closestDistance) {
            closestDistance = dist;
            goal = mob->getPos();
        }
    }

    works = closestDistance < std::numeric_limits<float>::max();
    if(!works)
        this->setEnabled(false);
}

void MobTP::onNormalTick(LocalPlayer* player) {
    if(!player || !works || tpDone)
        return;

    Vec3<float> cur = player->getPos();
    Vec3<float> delta(goal.x - cur.x, goal.y - cur.y, goal.z - cur.z);
    float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);

    if(dist <= arrivalThreshold) {
        resetAABB(player);
        tpDone = true;
        setDefaultTimer();
        player->displayClientMessage("[MobTP] Reached target");
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

void MobTP::onDisable() {
    setDefaultTimer();
    resetAABB(Game.getLocalPlayer());
}
