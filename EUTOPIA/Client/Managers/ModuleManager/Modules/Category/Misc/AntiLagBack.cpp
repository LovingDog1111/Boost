#include "AntiLagBack.h"

#include "..\Client\Managers\ModuleManager\Modules\Category\Combat\AntiHit.h"
#include "..\Utils\DrawUtil.h"

bool shadow;

AntiLagBack::AntiLagBack() : Module("AntiDesync", "Prevents desync", Category::MOVEMENT) {
    registerSetting(new BoolSetting("Show shadow", "Display the fake player", &shadow, false));
    currentX = 0.0f;
    currentY = 0.0f;
    currentZ = 0.0f;
}

void AntiLagBack::onEnable() {
    auto lp = Game.getLocalPlayer();
    if(lp) {
        Vec3<float> pos = lp->getPos();
        currentX = pos.x;
        currentY = pos.y;
        currentZ = pos.z;
    }
}

void AntiLagBack::onLevelRender() {
    auto lp = Game.getLocalPlayer();
    if(!lp)
        return;

    if(!AntiHit::isTeleporting) {
        Vec3<float> realPos = lp->getPos();
        float dx = realPos.x - currentX;
        float dz = realPos.z - currentZ;
        float dist = sqrt(dx * dx + dz * dz);

        auto updateCoord = [](float& current, float target, float offset = 0.0f) {
            float diff = (target + offset) - current;
            float step = std::max(-3.05f, std::min(3.05f, diff));
            current += step;
        };

        float curlX = 0.0f;
        float curlZ = 0.0f;

        if(dist > 0.01f) {
            float baseCurl = 2.0f;
            float dirX = dx / dist;
            float dirZ = dz / dist;

            float repelFactor = (dist < 2.0f) ? (2.0f / (dist + 0.2f)) : 1.0f;
            float finalIntensity = baseCurl * repelFactor;

            curlX = -dirZ * finalIntensity;
            curlZ = dirX * finalIntensity;
        }

        updateCoord(currentX, realPos.x, curlX);
        updateCoord(currentY, realPos.y);
        updateCoord(currentZ, realPos.z, curlZ); //a

        auto packet = MinecraftPacket::createPacket(PacketID::PlayerAuthInput);
        auto* authPkt = reinterpret_cast<PlayerAuthInputPacket*>(packet.get());

        authPkt->mPos = {currentX, currentY, currentZ};
        authPkt->mPosDelta = {0.f, 0.f, 0.f};
        authPkt->mRot = lp->getRotation();
        authPkt->mYHeadRot = lp->getActorHeadRotationComponent()->mHeadRot;
        authPkt->mInputMode = InputMode::Mouse;
        authPkt->mPlayMode = ClientPlayMode::Normal;
        authPkt->mInputData = AuthInputAction::NONE;
        authPkt->mClientTick = 0;

        Game.getPacketSender()->sendToServer(authPkt);
    }

    if(shadow) {
        float w = 0.6f / 2.0f;
        float h = 1.8f;
        AABB shadowBox;
        shadowBox.lower = {currentX - w, currentY, currentZ - w};
        shadowBox.upper = {currentX + w, currentY + h, currentZ + w};

        DrawUtil::drawBox3dFilled(shadowBox, UIColor(0, 0, 255, 40), UIColor(0, 0, 255, 135));
    }
}