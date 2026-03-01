#include "KillauraReachY.h"

#include <DrawUtil.h>
static float curX = 0.f;
static float curZ = 0.f;
static bool hInit = false;

#include "../../../../../../SDK/Network/PacketSender.h"
#include "../../../../../../Utils/Minecraft/TargetUtil.h"
static int hPausePhase = 0;  // 0 = off | 1 = forward | 2 = backward
static float hSaveX = 0.f;
static float hSaveZ = 0.f;

static float speedgoingup = 1.7f;
static float timervaluelol = 3000.f;
static float maxH = 3.f;
static bool swapping =
    false;  // true enquanto Y está se movendo para o target
            // Adicione essa variável estática no topo do arquivo, junto com as outras
static float targetTeleportY = 0.f;  // armazena o Y do alvo do F
static bool movingToTarget = false;  // indica se ainda está indo até o inimigo
static bool nofallpkt = false;       // indica se ainda está indo até o inimigo

void sendPosPacket(float x, float y, float z) {
    if(!Game.getPacketSender())
        return;
    // Cria a posição final com os valores recebidos
    Vec3 finalPos = {x, y, z};

    // Envio de pacote
    auto mep = MinecraftPacket::createPacket(PacketID::PlayerAuthInput);
    auto* pkt = reinterpret_cast<PlayerAuthInputPacket*>(mep.get());

    pkt->mPos = finalPos;
    pkt->mPosDelta = {0.f, 0.f, 0.f};
    pkt->mRot = {0.f, 0.f};
    pkt->mYHeadRot = 0.f;
    pkt->mAnalogMoveVector = {0.f, 0.f};
    pkt->mMove = {0.f, 0.f};
    pkt->mInteractRots = {0.f, 0.f};
    pkt->mCameraOrientation = {0.f, 0.f, 0.f};
    pkt->mRawMoveVector = {0.f, 0.f};
    pkt->mInputData = AuthInputAction::NONE;
    pkt->mInputMode = InputMode::Mouse;
    pkt->mPlayMode = ClientPlayMode::Normal;
    pkt->mNewInteractionModel = NewInteractionModel::Classic;
    pkt->mClientTick = 0;
    pkt->TicksAlive = 0;
    pkt->mItemUseTransaction = nullptr;
    pkt->mPredictedVehicle = 0;

    Game.getPacketSender()->sendToServer(pkt);
}
float getSurfaceYAtPos(const Vec3<float>& pos) {
    BlockSource* region = Game.getClientInstance()->getRegion();
    if(!region)
        return 0.f;

    int baseX = (int)floor(pos.x);
    int baseZ = (int)floor(pos.z);

    const int minWorldY = -64;
    const int maxWorldY = 320;

    float highestSurfaceY = (float)minWorldY;

    // Loop 3x3 no XZ
    for(int dx = -1; dx <= 1; dx++) {
        for(int dz = -1; dz <= 1; dz++) {
            int x = baseX + dx;
            int z = baseZ + dz;

            for(int y = maxWorldY; y >= minWorldY; y--) {
                Block* block = region->getBlock(BlockPos(x, y, z));
                if(!block || !block->blockLegacy)
                    continue;

                if(block->blockLegacy->blockId != 0) {
                    if((float)y > highestSurfaceY)
                        highestSurfaceY = (float)y;
                    break;
                }
            }
        }
    }

    return highestSurfaceY;
}
Vec3<float> getSurfacePosWithOrbit(const Vec3<float>& centerPos, float orbitAngle,
                                   float orbitRadius, float yOffset = 1.0f) {
    // calcula offset orbital
    float ox = cosf(orbitAngle) * orbitRadius;
    float oz = sinf(orbitAngle) * orbitRadius;

    Vec3<float> orbitXZ = {centerPos.x + ox, centerPos.y, centerPos.z + oz};

    float surfaceY = getSurfaceYAtPos(orbitXZ);

    return {orbitXZ.x, surfaceY + yOffset, orbitXZ.z};
}

float getSurfaceY(LocalPlayer* lp, Actor* closestTarget) {
    if(closestTarget) {
        return getSurfaceYAtPos(closestTarget->getPos());
    }

    if(lp) {
        return getSurfaceYAtPos(lp->getPos());
    }

    return 0.f;
}

static DWORD lastFPressMS = 0;       // última vez que F/auto foi acionado
static const DWORD fCooldown = 504;  // 504 ms de cooldown
static float orbitAngle = 0.0f;      // ângulo atual da órbita
static float orbitRadius = 3.0f;     // distância do target
static float orbitSpeed = 0.05f;     // velocidade angular

KillauraReachY::KillauraReachY()
    : Module("AntiHit-Test",
             "idk bro",
             Category::COMBAT) {
    registerSetting(new EnumSetting("Mode", "Attack mode type.", {"Press F", "Auto"}, &modeNZ, 0));
    registerSetting(new SliderSetting<float>("TeleportSpeed", "SPEED WHILE TELEPORTING TO TARGET",
                                             &speed, 2.f, 1.f, 10.f));
    registerSetting(new SliderSetting<float>("Max H speed", "Max H (speed horizontally limiter)",
                                             &maxH, 3.f, 0.1f, 10.f));
    registerSetting(new SliderSetting<float>("Orbit Radius", "Distance from target", &orbitRadius,
                                             3.f, 0.01f, 15.f));

    registerSetting(new SliderSetting<float>("Orbit Speed", "Angular orbit speed", &orbitSpeed,
                                             0.05f, 0.01f, 0.5f));

    registerSetting(new SliderSetting<float>("Speed-Y", "Speed WHILE GOING UP TO REACH TOP",
                                             &speedgoingup, 1.7f, 1.f, 10.f));
    registerSetting(new BoolSetting("Draw Box", "Debug orbit position", &drawBox, false));

    registerSetting(new BoolSetting(
        "NoFall Packet", "Sends classic noFall method Packet while reaching.", &nofallpkt, false));
}
static bool includeMobs = false;
static float debugX = 0.0f;
static float debugY = 0.0f;
static float debugZ = 0.0f;
static bool KSA_Mode =
    true;  // true = KSAmode (Y = 310), false = NoKSAmode (Y = localPlayer->getPos().y)
static bool G_wasPressed = false;  // para detectar apenas a transição do clique

std::string KillauraReachY::getModeText() {
    switch(mode) {
        case 0: {
            return "Nukkit";
            break;
        }
    }
    return "NULL";
}

static __int64 ms;
static DWORD lastMS = GetTickCount();
static __int64 timeMS = -1;
static DWORD getCurrentMs() {
    return GetTickCount();
}
static bool canPressF() {
    DWORD now = GetTickCount();
    if(now - lastFPressMS >= fCooldown) {
        lastFPressMS = now;
        return true;
    }
    return false;
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

void KillauraReachY::onEnable() {
    LocalPlayer* lp = Game.getLocalPlayer();
    if(!lp)
        return;

    curX = lp->getPos().x;
    curZ = lp->getPos().z;

    startY = lp->getPos().y;
    baseY = startY;
    valuePaipy = 0.f;
    canMove = 0;
}
static bool pkt2 = false;
static float pkt2f = 0.0f;
void KillauraReachY::onLevelRender() {
    ClientInstance* ci = GI::getClientInstance();
    MinecraftSim* mc = ci->minecraftSim;
    LocalPlayer* localPlayer = Game.getLocalPlayer();
    Level* level = localPlayer->level;
    float X = localPlayer->getPos().x;
    float Z = localPlayer->getPos().z;

    float Y = localPlayer->getPos().y;
    GameMode* gm = localPlayer->gamemode;
    if(localPlayer == nullptr)
        return;
    if(level == nullptr)
        return;
    if(mc != nullptr) {
    }

    // Sempre tenta encontrar o alvo mais próximo (mesmo fora do teleporte)
    Actor* closestTarget = nullptr;

    float minDistance = 300.0f;  // Distância máxima de ataque
    for(auto& actor : level->getRuntimeActorList()) {
        if(TargetUtil::isTargetValid(actor, includeMobs)) {
            float distance = actor->getEyePos().dist(localPlayer->getEyePos());
            if(distance < minDistance) {
                minDistance = distance;
                closestTarget = actor;
            }
        }
    }
    Vec3<float> surfacePos;

    if(closestTarget) {
        surfacePos = getSurfacePosWithOrbit(closestTarget->getPos(), orbitAngle, orbitRadius, 1.0f);
    } else {
        surfacePos = getSurfacePosWithOrbit(localPlayer->getPos(), orbitAngle, orbitRadius, 1.0f);
    }

    float surfaceY = surfacePos.y;
    float noKsaY = surfaceY + 16.0f;

    debugX = localPlayer->getPos().x;
    debugY = baseY - valuePaipy;
    debugZ = localPlayer->getPos().z;

    float targetY = KSA_Mode ? 329.9f : noKsaY;

    bool isFKeyPressed = (GetAsyncKeyState(0x46) & 0x8000) != 0;  // 0x46 = 'F'

    // Atualiza o tempo e a condição do teleporte
    auto currentTime = std::chrono::steady_clock::now();

    constexpr float yOffset = 4.0f;  // evita fall damage

    float distanceY = fabs(329.9f - targetY);

    // Swap com G só quando não estiver movendo
    bool isGPressed = (GetAsyncKeyState(0x47) & 0x8000) != 0;  // 0x47 = 'G'
    if(isGPressed && !G_wasPressed && !canMove) {
        baseY = 329;
    }
    G_wasPressed = isGPressed;

    // Substitua o bloco de F e atualização de baseY por:

    // Inicia teleporte quando F pressionado e não está movendo
    if(closestTarget) {
        // Se F pressionado manualmente ou modo Auto ativo
        bool activate = false;
        if(modeNZ == 0) {  // Press F
            activate = isFKeyPressed;
        } else if(modeNZ == 1) {  // Auto
            activate = true;      // sempre tenta
        }

        if(!canMove && activate && canPressF()) {
            // >>> NOVA REGRA: se alvo estiver abaixo da superfície, NÃO dar TP
            if(closestTarget->getPos().y < surfaceY) {
                // cancela totalmente
                canMove = 0;
                movingToTarget = false;
            } else {
                canMove = 1;
                targetTeleportY = closestTarget->getPos().y + yOffset;
                movingToTarget = true;
            }
        }
    }

    // Atualiza baseY suavemente
    if(canMove && hPausePhase == 0) {
        float currentTargetY = movingToTarget ? targetTeleportY : (KSA_Mode ? 329.9f : noKsaY);

        float step = std::min(speedgoingup, fabs(currentTargetY - baseY));
        if(baseY < currentTargetY)
            baseY += step;
        else if(baseY > currentTargetY)
            baseY -= step;

        if(fabs(baseY - currentTargetY) < 0.01f) {
            baseY = currentTargetY;

            if(movingToTarget) {
                movingToTarget = false;  // chegou no inimigo
                                         // >>> pausa horizontal de 2 ticks

            } else {
                canMove = 0;  // teleporte concluído, agora G pode trocar o modo
            }
        }
    }
    // Se não estamos no teleporte, baseY ainda deve subir/descer suavemente para o Y do modo
    if(canMove == 0 && hPausePhase == 0) {
        float targetModeY = KSA_Mode ? 329.9f : noKsaY;

        float step = std::min(speedgoingup, fabs(targetModeY - baseY));

        if(baseY < targetModeY) {
            baseY += step;
        }

        else if(baseY > targetModeY) {
            baseY -= step;
        }
        if(fabs(baseY - targetModeY) < 0.01f)
            baseY = targetModeY;
    }
    if(baseY > 329.f)
        baseY = 329.f;

    Y = baseY;

    // decide modo APENAS quando NÃO está teleportando
    if(closestTarget) {
        LocalPlayer* lp = Game.getLocalPlayer();
        if(lp) {
            float playerY = noKsaY;
            float targetY = closestTarget->getPos().y;

            float midY = (playerY + 329.f) * 0.5f;

            if(targetY < midY) {
                KSA_Mode = true;  // NoKSA
            } else {
                KSA_Mode = false;  // KSA
            }
        }
    }

    if(!closestTarget) {
        // KSA_Mode = false;
    }

    float targetX;
    float targetZ;
    orbitAngle += orbitSpeed;

    if(orbitAngle > 6.283185f)  // 2*pi
        orbitAngle -= 6.283185f;

    if(closestTarget) {
        Vec3<float> tpos = closestTarget->getPos();

        targetX = tpos.x + cosf(orbitAngle) * orbitRadius;
        targetZ = tpos.z + sinf(orbitAngle) * orbitRadius;
    } else {
        Vec3<float> lpos = localPlayer->getPos();
        targetX = lpos.x + cosf(orbitAngle) * orbitRadius;
        targetZ = lpos.z + sinf(orbitAngle) * orbitRadius;
    }

    if(hPausePhase != 0 && closestTarget) {
        Vec3<float> tpos = closestTarget->getPos();

        float dx = tpos.x - hSaveX;
        float dz = tpos.z - hSaveZ;
        float dist = sqrt(dx * dx + dz * dz);

        if(dist > 0.001f) {
            float nx = dx / dist;
            float nz = dz / dist;

            if(hPausePhase == 1) {
                curX = hSaveX + nx * (speedgoingup / 1.4);
                curZ = hSaveZ + nz * (speedgoingup / 1.4);
                hPausePhase = 2;
            } else if(hPausePhase == 2) {
                curX = hSaveX;
                curZ = hSaveZ;
                hPausePhase = 0;
            }
        }

        Y = baseY;  // pausa Y por 2 ticks
    }

    if(hPausePhase == 0) {
        float dx = targetX - curX;
        float dz = targetZ - curZ;

        float dist = sqrt(dx * dx + dz * dz);

        if(dist > 0.f) {
            float step = std::min(maxH, dist);
            float nx = dx / dist;
            float nz = dz / dist;

            curX += nx * step;
            curZ += nz * step;
        }
    }

    debugX = curX;
    debugY = Y;
    debugZ = curZ;

    // Envio de pacote
    auto mep = MinecraftPacket::createPacket(PacketID::PlayerAuthInput);
    auto* pkt = reinterpret_cast<PlayerAuthInputPacket*>(mep.get());
    pkt->mPos = {curX, Y, curZ};
    pkt->mPosDelta = {0.f, 0.f, 0.f};
    pkt->mRot = {0.f, 0.f};
    pkt->mYHeadRot = 0.f;
    pkt->mAnalogMoveVector = {0.f, 0.f};
    pkt->mMove = {0.f, 0.f};
    pkt->mInteractRots = {0.f, 0.f};
    pkt->mCameraOrientation = {0.f, 0.f, 0.f};
    pkt->mRawMoveVector = {0.f, 0.f};
    pkt->mInputData = AuthInputAction::NONE;
    pkt->mInputMode = InputMode::Mouse;
    pkt->mPlayMode = ClientPlayMode::Normal;
    pkt->mNewInteractionModel = NewInteractionModel::Classic;
    pkt->mClientTick = 0;
    pkt->TicksAlive = 0;
    pkt->mItemUseTransaction = nullptr;
    pkt->mPredictedVehicle = 0;

    Game.getPacketSender()->sendToServer(pkt);

    if(pkt2 == false) {
        pkt2 = true;
        pkt2f = 0.1f;
    } else {
        pkt2 = false;
        pkt2f = 0.0f;
    }

    auto mep2 = MinecraftPacket::createPacket(PacketID::PlayerAuthInput);
    auto* pkt2 = reinterpret_cast<PlayerAuthInputPacket*>(mep2.get());
    pkt2->mPos = {curX, Y + pkt2f, curZ};
    pkt2->mPosDelta = {0.f, 0.f, 0.f};
    pkt2->mRot = {0.f, 0.f};
    pkt2->mYHeadRot = 0.f;

    pkt2->mAnalogMoveVector = {0.f, 0.f};
    pkt2->mMove = {0.f, 0.f};
    pkt2->mInteractRots = {0.f, 0.f};
    pkt2->mCameraOrientation = {0.f, 0.f, 0.f};
    pkt2->mRawMoveVector = {0.f, 0.f};
    pkt2->mInputData = AuthInputAction::NONE;
    pkt2->mInputMode = InputMode::Mouse;
    pkt2->mPlayMode = ClientPlayMode::Normal;
    pkt2->mNewInteractionModel = NewInteractionModel::Classic;
    pkt2->mClientTick = 0;
    pkt2->TicksAlive = 0;
    pkt2->mItemUseTransaction = nullptr;
    pkt2->mPredictedVehicle = 0;

    if(nofallpkt) {
        Game.getPacketSender()->sendToServer(pkt2);
    }

    // *** Bloco de ataque e swing fora da condição de teleporte ***
    // Esse bloco será executado sempre que um packet do tipo "PlayerAuthInputPacket" chegar,
    // independente do teleporte.

    if(closestTarget != nullptr && canMove == 1) {
        gm->attack(closestTarget);  // Ataque
        localPlayer->swing();
    }

    if(!drawBox)
        return;

    const float halfWidth = 0.4f;
    const float height = 2.f;

    AABB debugBox;
    debugBox.lower.x = debugX - halfWidth;
    debugBox.lower.y = debugY;
    debugBox.lower.z = debugZ - halfWidth;

    debugBox.upper.x = debugX + halfWidth;
    debugBox.upper.y = debugY + height;
    debugBox.upper.z = debugZ + halfWidth;

    DrawUtil::drawBox3dFilled(debugBox, UIColor(255, 0, 0, 60),  // fill
                              UIColor(255, 0, 0, 255)            // line
    );

    if(drawBox) {
        float boxX;
        float boxZ;

        if(closestTarget) {
            Vec3<float> tpos = closestTarget->getPos();
            boxX = tpos.x;
            boxZ = tpos.z;
        } else {
            boxX = X;  // localPlayer->getPos().x
            boxZ = Z;  // localPlayer->getPos().z
        }

        AABB surfaceBox;
        surfaceBox.lower = {surfacePos.x - 0.5f, surfacePos.y, surfacePos.z - 0.5f};
        surfaceBox.upper = {surfacePos.x + 0.5f, surfacePos.y + 0.1f, surfacePos.z + 0.5f};

        DrawUtil::drawBox3dFilled(surfaceBox, UIColor(0, 255, 0, 60),  // fill
                                  UIColor(0, 255, 0, 255)              // outline
        );
    }
}

void KillauraReachY::onDisable() {
    ClientInstance* ci = GI::getClientInstance();

    MinecraftSim* mc = ci->minecraftSim;
}