#include "KillauraReach.h"

#include "../../../ModuleManager.h"
#include "..\Utils\Minecraft\TargetUtil.h"
#include "..\Utils\Minecraft\Utils.h"

KillauraReach::KillauraReach()
    : Module("KillauraReach", "Hit your enemies from a far away place.", Category::COMBAT) {
    registerSetting(new EnumSetting("Mode", "Attack mode type.", {"Press F", "Auto", "Left Click"},
                                    &modeNZ, 0));
    registerSetting(new SliderSetting<float>("Speed", "teleport speed", &speed, 1.f, 1.f, 3.2f));
}
Vec3<float> theOldPos;
static __int64 ms;
static DWORD lastMS = GetTickCount();
static __int64 timeMS = -1;

static DWORD getCurrentMs() {
    return GetTickCount();
}

static __int64 getElapsedTime() {
    return getCurrentMs() - ms;
}

static void resetTime() {
    lastMS = getCurrentMs();
    timeMS = getCurrentMs();
}

static bool hasTimedElapsed(__int64 time, bool reset) {
    if(getCurrentMs() - lastMS > time) {
        if(reset)
            resetTime();
        return true;
    }
    return false;
}

void KillauraReach::onEnable() {
    valuePaipy = 0.0f;
    canMove = 0;
}

void KillauraReach::onSendPacket(Packet* packet) {
    auto client = Game.getClientInstance();
    if(!client)
        return;

    LocalPlayer* localPlayer = client->getLocalPlayer();
    if(!localPlayer)
        return;

    theOldPos = localPlayer->getPos();

    Minecraft* mc = client->minecraft;
    if(!mc || !mc->minecraftTimer)
        return;

    Level* level = localPlayer->level;
    if(!level)
        return;

    GameMode* gm = localPlayer->gamemode;
    if(!gm)
        return;

    Utils::resetTimer();

    bool isFKeyPressed = (GetAsyncKeyState(0x46) & 0x8000) != 0;
    auto currentTime = std::chrono::steady_clock::now();

    if(modeNZ == 2) {
        if(Game.isLeftClickDown() && canMove == 0 && (currentTime - lastTeleportTime) >= minDelay) {
            if(!((GetAsyncKeyState('W') & 0x8000) || (GetAsyncKeyState('A') & 0x8000) ||
                 (GetAsyncKeyState('S') & 0x8000) || (GetAsyncKeyState('D') & 0x8000) ||
                 (GetAsyncKeyState(VK_SPACE) & 0x8000) || (GetAsyncKeyState(VK_SHIFT) & 0x8000))) {
                canMove = 1;
                lastTeleportTime = currentTime;
            }
        }
    }

    if(modeNZ == 0) {
        if(isFKeyPressed && canMove == 0 && (currentTime - lastTeleportTime) >= minDelay) {
            if(!((GetAsyncKeyState('W') & 0x8000) || (GetAsyncKeyState('A') & 0x8000) ||
                 (GetAsyncKeyState('S') & 0x8000) || (GetAsyncKeyState('D') & 0x8000) ||
                 (GetAsyncKeyState(VK_SPACE) & 0x8000) || (GetAsyncKeyState(VK_SHIFT) & 0x8000))) {
                canMove = 1;
                lastTeleportTime = currentTime;
            }
        }
    }

    if(modeNZ == 1) {
        if(canMove == 0 && (currentTime - lastTeleportTime) >= minDelay) {
            if(!((GetAsyncKeyState('W') & 0x8000) || (GetAsyncKeyState('A') & 0x8000) ||
                 (GetAsyncKeyState('S') & 0x8000) || (GetAsyncKeyState('D') & 0x8000) ||
                 (GetAsyncKeyState(VK_SPACE) & 0x8000) || (GetAsyncKeyState(VK_SHIFT) & 0x8000))) {
                canMove = 1;
                lastTeleportTime = currentTime;
            }
        }
    }

    Actor* closestTarget = nullptr;
    float minDistance = 300.0f;

    const auto& actors = level->getRuntimeActorList();
    for(auto& actor : actors) {
        if(actor && TargetUtil::isTargetValid(actor, false, 999.f)) {
            float distance = actor->getEyePos().dist(localPlayer->getEyePos());
            if(distance < minDistance) {
                minDistance = distance;
                closestTarget = actor;
            }
        }
    }

    if(!closestTarget) {
        canMove = 0;
        Utils::resetTimer();
        return;
    }

    if(canMove == 1) {
        float distanceToClosestTarget = localPlayer->getPos().dist(Vec3<float>(
            closestTarget->getPos().x, localPlayer->getPos().y, closestTarget->getPos().z));

        if(valuePaipy >= distanceToClosestTarget) {
            reverseTrick = 1;
        } else if(valuePaipy <= 0) {
            reverseTrick = 0;
        }

        if(reverseTrick == 1) {
            Utils::setTimer(999999.f);
            valuePaipy -= speed;
            if(valuePaipy <= 0) {
                reverseTrick = 0;
                canMove = 0;
                Utils::resetTimer();
            }
        } else {
            Utils::setTimer(999999.f);
            valuePaipy += speed;
            if(valuePaipy >= distanceToClosestTarget) {
                reverseTrick = 1;
                Utils::resetTimer();
            }
        }

        if(packet && packet->getName() == "PlayerAuthInputPacket") {
            auto paip = static_cast<PlayerAuthInputPacket*>(packet);
            if(paip) {
                Vec3<float> direction =
                    (closestTarget->getPos() - localPlayer->getPos()).normalize();
                paip->mPos.x += direction.x * valuePaipy;
                paip->mPos.z += direction.z * valuePaipy;
            }
        }
    }

    if(packet && packet->getName() == "PlayerAuthInputPacket") {
        auto killauraModule = ModuleManager::getModule<KillAuraH>();
        if(killauraModule && !killauraModule->isEnabled()) {
            localPlayer->swing();
            gm->attack(closestTarget);
        }
    }
}

void KillauraReach::onDisable() {
    auto client = Game.getClientInstance();
    Utils::resetTimer();

    LocalPlayer* player = client->getLocalPlayer();
    if(!player)
        return;



    Vec3<float> delta = theOldPos.sub(player->getPos());
    float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    if(dist < 0.001f)
        return;

    float step = std::min(1.5f, dist);
    Vec3<float> movement = delta * (step / dist);
    Utils::setTimer(100.f);
    player->lerpMotion(movement);
    AABB a = player->getAABB(true);
    a.lower = a.lower.add(movement);
    a.upper = a.upper.add(movement);
    player->setAABB(a);
    Utils::resetTimer();
}
