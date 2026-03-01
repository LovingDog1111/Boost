#include "LbReachV2.h"

#include <DrawUtil.h>

#include <chrono>

#include "../../../../../../SDK/Network/PacketSender.h"
#include "../../../../../../Utils/Minecraft/TargetUtil.h"

bool LbReachV2::isTeleporting = false;

LbReachV2::LbReachV2()
    : Module("LbReachV2", "Remake of lbreach", Category::COMBAT) {
    registerSetting(
        new SliderSetting<float>("Reach", "Total Reach", &longReach, 50.0f, 5.0f, 150.0f));
    registerSetting(new EnumSetting("Hit mode", "Activation", {"Press F", "Auto"}, &hitmode, 0));
}

void LbReachV2::onEnable() {
    isTeleporting = false;
}

void LbReachV2::onDisable() {
    isTeleporting = false;
}

void LbReachV2::onLevelRender() {
    auto lp = Game.getLocalPlayer();
    if(!lp || !lp->level)
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
            shouldAttack = true;
    } else {
        shouldAttack = true;
    }

    if(shouldAttack && target) {
        static auto lastAttackTime = std::chrono::steady_clock::now();
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastAttackTime)
                .count();

        if(elapsed < 380)
            return;

        isTeleporting = true;
        lastAttackTime = currentTime;

        Vec3<float> start = lp->getPos();
        Vec3<float> end = target->getPos();
        float dist = start.dist(end);

        float step = 12.0f;
        int packets = (int)(dist / step);
        if(packets < 1)
            packets = 1;

        for(int i = 1; i <= packets; i++) {
            float r = (float)i / (float)packets;
            sendPos({start.x + (end.x - start.x) * r, start.y + (end.y - start.y) * r,
                     start.z + (end.z - start.z) * r});
        }

        sendPos(end);

        lp->gamemode->attack(target);
        lp->swing();

        for(int i = packets; i >= 0; i--) {
            float r = (float)i / (float)packets;
            sendPos({start.x + (end.x - start.x) * r, start.y + (end.y - start.y) * r,
                     start.z + (end.z - start.z) * r});
        }

        isTeleporting = false;
    }
}

void LbReachV2::sendPos(Vec3<float> p) {
    auto lp = Game.getLocalPlayer();
    if(!lp)
        return;

    auto sender = Game.getPacketSender();
    if(!sender)
        return;

    auto authPkt = MinecraftPacket::createPacket(PacketID::PlayerAuthInput);
    auto* auth = reinterpret_cast<PlayerAuthInputPacket*>(authPkt.get());

    auth->mPos = p;
    auth->mPosDelta = {0.f, 0.f, 0.f};
    auth->mRot = lp->getRotation();
    auth->mYHeadRot = lp->getActorHeadRotationComponent()->mHeadRot;
    auth->mInputMode = InputMode::Mouse;
    auth->mPlayMode = ClientPlayMode::Normal;
    auth->mInputData = AuthInputAction::NONE;
    auth->mClientTick = 0;

    sender->sendToServer(auth);

    auto movePkt = MinecraftPacket::createPacket(PacketID::MovePlayer);
    auto* move = reinterpret_cast<MovePlayerPacket*>(movePkt.get());
    move->mPos = p;
    move->mRot = lp->getRotation();
    move->mOnGround = true;
    sender->sendToServer(move);

    static bool toggle = false;
    toggle = !toggle;

    auto authPkt2 = MinecraftPacket::createPacket(PacketID::PlayerAuthInput);
    auto* auth2 = reinterpret_cast<PlayerAuthInputPacket*>(authPkt2.get());
    auth2->mPos = {p.x, p.y + (toggle ? 0.03f : 0.00f), p.z};
    auth2->mPosDelta = {0.f, 0.f, 0.f};
    auth2->mRot = lp->getRotation();
    auth2->mYHeadRot = lp->getActorHeadRotationComponent()->mHeadRot;
    auth2->mInputData = AuthInputAction::NONE;
    auth2->mInputMode = InputMode::Mouse;
    auth2->mPlayMode = ClientPlayMode::Normal;
    auth2->mClientTick = 0;

    sender->sendToServer(auth2);
}