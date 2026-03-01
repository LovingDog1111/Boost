#include "AntiHit.h"

#include <DrawUtil.h>

#include "../../../../../../SDK/Network/PacketSender.h"
#include "../../../../../../Utils/Minecraft/TargetUtil.h"

int hitmode = 0;

static float longReach = 50.0f;
float delayhit = 250.f;

AntiHit::AntiHit() : Module("LbReach", "Hit enemies from far away", Category::COMBAT) {
    registerSetting(
        new SliderSetting<float>("Reach", "Total Reach", &longReach, 50.0f, 5.0f, 150.0f));
    registerSetting(
        new EnumSetting("Hit mode", "How to hit target", {"Press F", "Auto"}, &hitmode, 0));
    registerSetting(new SliderSetting<float>("Delay (ms)", "Hit delay (lower delay can cause kicks)",
                                             &delayhit, 250.f, 100.f, 250.f));
}

bool AntiHit::isTeleporting = false;

void AntiHit::onEnable() {
    AntiHit::isTeleporting = false;
}

void AntiHit::onLevelRender() {
    auto lp = Game.getLocalPlayer();
    if(!lp)
        return;

    Actor* target = nullptr;
    float scan = longReach;
    for(auto& actor : lp->level->getRuntimeActorList()) {
        if(TargetUtil::isTargetValid(actor, false)) {
            float d = actor->getPos().dist(lp->getPos());
            if(d < scan) {
                scan = d;
                target = actor;
            }
        }
    }

    bool shouldAttack = false;
    if(hitmode == 0) {
        if(GetAsyncKeyState(0x46) & 0x8000)
            shouldAttack = true;  // Press F
    } else {
        shouldAttack = true;  // Auto mode
    }

    if(shouldAttack && target) {

        static auto lastAttackTime = std::chrono::steady_clock::now();
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastAttackTime)
                .count();

        if(elapsed < delayhit) {
            return;
        }

        AntiHit::isTeleporting = true;
        lastAttackTime = currentTime;
        Vec3<float> start = lp->getPos();
        Vec3<float> end = target->getPos();

        float dist = start.dist(end);
        float step = 3.f;
        int packets = (int)(dist / step);
        Utils::setTimer(50000.f);
        for(int i = 1; i <= packets; i++) {
            float r = (float)i / (float)packets;
            sendPos({start.x + (end.x - start.x) * r, start.y + (end.y - start.y) * r,
                     start.z + (end.z - start.z) * r});
        }

        sendPos(end);

        Utils::resetTimer();

        lp->gamemode->attack(target);
        lp->swing();
        Utils::setTimer(50000.f);

        for(int i = packets; i >= 0; i--) {
            float r = (float)i / (float)packets;
            sendPos({start.x + (end.x - start.x) * r, start.y + (end.y - start.y) * r,
                     start.z + (end.z - start.z) * r});
        }

        Utils::resetTimer();
        AntiHit::isTeleporting = false;
    }
}

void AntiHit::sendPos(Vec3<float> p) {
    auto lp = Game.getLocalPlayer();
    if(!lp)
        return;

    auto mep = MinecraftPacket::createPacket(PacketID::PlayerAuthInput);
    auto* pkt = reinterpret_cast<PlayerAuthInputPacket*>(mep.get());

    pkt->mPos = p;
    pkt->mPosDelta = {0, 0, 0};
    pkt->mRot = lp->getRotation();
    pkt->mYHeadRot = lp->getActorHeadRotationComponent()->mHeadRot;
    pkt->mInputMode = InputMode::Mouse;
    pkt->mPlayMode = ClientPlayMode::Normal;
    pkt->mInputData = AuthInputAction::NONE;
    pkt->mClientTick = 0;

    Game.getPacketSender()->sendToServer(pkt);

    // no fall
    auto mep2 = MinecraftPacket::createPacket(PacketID::PlayerAuthInput);
    auto* pkt2 = reinterpret_cast<PlayerAuthInputPacket*>(mep2.get());

    auto move = MinecraftPacket::createPacket(PacketID::MovePlayer);
    auto* movepkt = reinterpret_cast<MovePlayerPacket*>(move.get());

    movepkt->mOnGround = true;

    Game.getPacketSender()->sendToServer(movepkt);

    pkt2->mPos = {p.x, p.y + 0.1f, p.z};
    pkt2->mPosDelta = {0.f, 0.f, 0.f};
    pkt2->mRot = lp->getRotation();
    pkt2->mYHeadRot = lp->getActorHeadRotationComponent()->mHeadRot;
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

    Game.getPacketSender()->sendToServer(pkt2);
}