#include "NoFall.h"



NoFall::NoFall() : Module("NoFall", "Prevents fall damage", Category::WORLD) {
    registerSetting(new EnumSetting("Server", "change mode",
                                    {"Sentinel", "BDS", "Lifeboat", "OnGround"}, &Mode, 0));
}

void NoFall::onEnable() {
    altToggle = false;
    lastY = 0.f;
}

void NoFall::onDisable() {
    altToggle = false;
    lastY = 0.f;
}

void NoFall::onSendPacket(Packet* packet) {
    auto* player = Game.getLocalPlayer();
    if(!player)
        return;

    if(packet->getId() == PacketID::PlayerAuthInput) {
        auto* paip = static_cast<PlayerAuthInputPacket*>(packet);

        if(Mode == 0) {
            altToggle = !altToggle;
            if(altToggle)
                return;
            paip->mPosDelta.y = -0.0784f;
        }

        else if(Mode == 1) {
            paip->mInputData |= AuthInputAction::START_GLIDING;
            paip->mInputData &= ~AuthInputAction::STOP_GLIDING;
            paip->mPosDelta = {0.f, 0.f, 0.f};
        }

        else if(Mode == 2) {
            if(!player)
                return;

            altToggle = !altToggle;
            if(altToggle)
                lastY = paip->mPos.y;
            else {
                paip->mPos.y = lastY + 0.1f;
                paip->mPosDelta = {0.f, 0.f, 0.f};
                paip->removeMovingInput();
            }
        }
    }
    
    if(packet->getId() == PacketID::MovePlayer && Mode == 3) {
        auto* mpp = static_cast<MovePlayerPacket*>(packet);
        mpp->mOnGround = true;
    }
}
