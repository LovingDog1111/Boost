
#include "KillauraReachY2.h"

#include <DrawUtil.h>

#include "../../../../../../SDK/Network/PacketSender.h"
#include "../../../../../../Utils/Minecraft/TargetUtil.h"
static bool KSA_Mode = false;
static float speedgoingup = 1.7f;
static float timervaluelol = 3000.f;
static float maxH = 3.f;
static bool swapping =
    false;  // true enquanto Y está se movendo para o target
            // Adicione essa variável estática no topo do arquivo, junto com as outras
static float targetTeleportY = 0.f;  // armazena o Y do alvo do F
static bool movingToTarget = false;  // indica se ainda está indo até o inimigo
static bool nofallpkt = false;       // indica se ainda está indo até o inimigo

static DWORD lastFPressMS = 0;       // última vez que F/auto foi acionado
static const DWORD fCooldown = 504;  // 504 ms de cooldown
static int tpTickState = 0;          // 0=idle, 1=go, 2=stay, 3=return
static float returnY = 0.f;          // Y original antes do F
static bool needInitialSync = false;

void sendPosPacket2(float x, float y, float z) {
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

void sendKsaMode2() {
    LocalPlayer* lp = Game.getLocalPlayer();
    if(!lp)
        return;

    Vec3<float> pos = lp->getPos();

    float x = pos.x;
    float z = pos.z;

    float startY = pos.y;
    float endY = 310.f;

    const float step = speedgoingup;

    // Decide direção (subir ou descer)
    float dir = (endY > startY) ? step : -step;

    // Envia os packets em etapas de 5 blocos no eixo Y
    for(float y = startY; (dir > 0 ? y < endY : y > endY); y += dir) {
        sendPosPacket2(x, y, z);
    }

    // Garante que o último packet seja exatamente em Y = 310
    sendPosPacket2(x, endY, z);
}

void sendNoKsaMode2() {
    LocalPlayer* lp = Game.getLocalPlayer();
    if(!lp)
        return;

    Vec3<float> pos = lp->getPos();

    float x = pos.x;
    float z = pos.z;

    float startY = 310.f;
    float endY = pos.y;

    const float step = speedgoingup;

    // Direção: sempre descendo
    float dir = (endY < startY) ? -step : step;

    // Envia os packets em etapas de 5 blocos no eixo Y
    for(float y = startY; (dir < 0 ? y > endY : y < endY); y += dir) {
        sendPosPacket2(x, y, z);
    }

    // Garante que o último packet seja exatamente no Y original
    sendPosPacket2(x, endY, z);
}

void sendTPtoTarget(Actor* closestTarget) {
    if(!closestTarget)
        return;

    LocalPlayer* lp = Game.getLocalPlayer();
    if(!lp)
        return;

    Vec3<float> playerPos = lp->getPos();
    Vec3<float> targetPos = closestTarget->getPos();

    float x = playerPos.x;
    float z = playerPos.z;

    float startY = KSA_Mode ? 310.f : playerPos.y;
    float endY = targetPos.y;

    const float step = speedgoingup;

    // Decide direção (subir ou descer)
    float dir = (endY > startY) ? step : -step;

    // Envia os packets em etapas de 5 blocos no eixo Y
    for(float y = startY; (dir > 0 ? y < endY : y > endY); y += dir) {
        sendPosPacket2(x, y, z);
    }

    // Garante posição final exata no Y do alvo
    sendPosPacket2(x, endY, z);
}

void sendTPback(Actor* closestTarget) {
    if(!closestTarget)
        return;

    LocalPlayer* lp = Game.getLocalPlayer();
    if(!lp)
        return;
    
    Vec3<float> playerPos = lp->getPos();
    Vec3<float> targetPos = closestTarget->getPos();

    float x = playerPos.x;
    float z = playerPos.z;

    // Origem: Y do alvo
    float startY = targetPos.y;

    // Destino:
    // - KSA_Mode → volta para Y = 310
    // - NoKSA    → volta para Y original do player
    float endY = KSA_Mode ? 310.f : playerPos.y;

    const float step = speedgoingup;

    // Decide direção (subir ou descer)
    float dir = (endY > startY) ? step : -step;

    // Envia os packets em etapas de 5 blocos no eixo Y
    for(float y = startY; (dir > 0 ? y < endY : y > endY); y += dir) {
        sendPosPacket2(x, y, z);
    }

    // Garante posição final exata
    sendPosPacket2(x, endY, z);
}

KillauraReachY2::KillauraReachY2()
    : Module("KillauraReachY2",
             "(Press F to hit) Hit ur enemies far (for normal SM)",
             Category::COMBAT) {
    registerSetting(new EnumSetting("Mode", "Attack mode type.", {"Press F", "Auto"}, &modeNZ, 0));
    registerSetting(new SliderSetting<float>("TeleportSpeed", "SPEED WHILE TELEPORTING TO TARGET",
                                             &speed, 2.f, 1.f, 10.f));
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

static bool G_wasPressed = false;  // para detectar apenas a transição do clique

std::string KillauraReachY2::getModeText() {
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

void KillauraReachY2::onEnable() {
    LocalPlayer* lp = Game.getLocalPlayer();
    if(!lp)
        return;

    startY = lp->getPos().y;
    baseY = startY;
    valuePaipy = 0.f;
    needInitialSync = KSA_Mode;
    canMove = 0;
}
static bool pkt2 = false;
static float pkt2f = 0.0f;
void KillauraReachY2::onLevelRender() {
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

    debugX = localPlayer->getPos().x;
    debugY = baseY - valuePaipy;
    debugZ = localPlayer->getPos().z;

    float targetY = KSA_Mode ? 310.f : localPlayer->getPos().y;

    bool isFKeyPressed = (GetAsyncKeyState(0x46) & 0x8000) != 0;  // 0x46 = 'F'

    // Atualiza o tempo e a condição do teleporte
    auto currentTime = std::chrono::steady_clock::now();

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

    constexpr float yOffset = 4.0f;  // evita fall damage

    float distanceY = fabs(310.f - targetY);

    // Swap com G só quando não estiver movendo

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
            canMove = 1;
            tpTickState = 1;  // começa o ciclo
        }
    }

    if(canMove && closestTarget) {
        float x = localPlayer->getPos().x;
        float z = localPlayer->getPos().z;
        float targetY = closestTarget->getPos().y;

        switch(tpTickState) {
            case 1:
                // TICK 1 → vai até o alvo
                sendTPtoTarget(closestTarget);
                tpTickState = 2;
                break;

            case 2:
                // TICK 2 → fica 1 tick no alvo
                sendPosPacket2(x, targetY, z);
                tpTickState = 3;
                break;

            case 3:
                // TICK 3 → volta
                sendTPback(closestTarget);

                canMove = 0;
                tpTickState = 0;
                break;
        }
    }

    // decide modo APENAS quando NÃO está teleportando
    if(canMove == 0 && tpTickState == 0 && closestTarget) {
        LocalPlayer* lp = Game.getLocalPlayer();
        if(lp) {
            float playerY = lp->getPos().y;
            float targetY = closestTarget->getPos().y;

            // meio entre playerY e 310
            float midY = (playerY + 310.f) * 0.5f;

            if(targetY < midY) {
                KSA_Mode = true;  // NoKSA
            } else {
                KSA_Mode = false;  // KSA
            }
        }
    }
    // memoriza estado anterior
    static bool lastKSA_Mode = KSA_Mode;

    // detecta troca de estado (false -> true ou true -> false)
    if(KSA_Mode != lastKSA_Mode) {
        if(KSA_Mode) {
            // entrou em KSA
            sendKsaMode2();
        } else {
            // entrou em NoKSA
            sendNoKsaMode2();
        }

        // atualiza estado salvo
        lastKSA_Mode = KSA_Mode;
    }

    float targetX = localPlayer->getPos().x;
    float targetZ = localPlayer->getPos().z;

    float deltaX = targetX - X;
    float deltaZ = targetZ - Z;
    float distH = sqrt(deltaX * deltaX + deltaZ * deltaZ);

    if(distH > maxH && distH > 0.f) {
        float ratio = maxH / distH;
        deltaX *= ratio;
        deltaZ *= ratio;
    }

    X += deltaX;
    Z += deltaZ;
    // Sync inicial para o server entender o "caminho"
    // Sync inicial APENAS para KSA
    if(needInitialSync && !canMove && tpTickState == 0) {
        sendKsaMode2();  // só sobe
        needInitialSync = false;
    }

    if(!canMove && tpTickState != 2)
        sendPosPacket2(localPlayer->getPos().x, KSA_Mode ? 310.f : localPlayer->getPos().y,
                       localPlayer->getPos().z);

    debugX = X;
    debugY = Y;
    debugZ = Z;

    // *** Bloco de ataque e swing fora da condição de teleporte ***
    // Esse bloco será executado sempre que um packet do tipo "PlayerAuthInputPacket" chegar,
    // independente do teleporte.

    if(closestTarget != nullptr && canMove == 1) {
        gm->attack(closestTarget);  // Ataque
        localPlayer->swing();
    }

    if(!drawBox)
        return;

    const float halfWidth = 1.f;
    const float height = 2.1f;

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
}

void KillauraReachY2::onDisable() {
    ClientInstance* ci = GI::getClientInstance();

    MinecraftSim* mc = ci->minecraftSim;
}