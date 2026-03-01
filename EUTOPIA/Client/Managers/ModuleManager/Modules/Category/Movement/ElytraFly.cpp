#include "ElytraFly.h"
static float Speedy = 4.0f;
bool noclip;
bool elytraspoofing;

ElytraFly::ElytraFly() : Module("ElytraFly", "Elytra flight", Category::MOVEMENT) {
    registerSetting(new KeybindSetting("Up Key", "Key for up", &upKey, 0));
    registerSetting(new KeybindSetting("Down Key", "Key for down", &downKey, 0));
    registerSetting(new SliderSetting<float>("Speed", "NULL", &Speed, 25.f, 0.f, 50.f));
    registerSetting(new SliderSetting<float>("Vertical Speed", "NULL", &Speedy, 25.f, 0.f, 50.f));
    registerSetting(new BoolSetting("Noclip", "Noclip flight", &noclip, true));
    registerSetting(new BoolSetting("Auto Take Off", "Auto take off. Useful for noclipping.",
                                    &elytraspoofing, true));
}

static Vec2<float> getMotion(float playerYaw, float speed) {
    bool w = Game.isKeyDown('W');
    bool a = Game.isKeyDown('A');
    bool s = Game.isKeyDown('S');
    bool d = Game.isKeyDown('D');

    if(w) {
        if(!a && !d)
            playerYaw += 90.0f;
        if(a)
            playerYaw += 45.0f;
        else if(d)
            playerYaw += 135.0f;
    } else if(s) {
        if(!a && !d)
            playerYaw -= 90.0f;
        if(a)
            playerYaw -= 45.0f;
        else if(d)
            playerYaw -= 135.0f;
    } else {
        if(!a && d)
            playerYaw += 180.0f;
    }

    float calcYaw = playerYaw * 0.01745329251f;
    return {cos(calcYaw) * speed / 10.0f, sin(calcYaw) * speed / 10.0f};
}

bool iszoving(bool jumpCheck) {
    if(!Game.canUseMoveKeys())
        return false;
    if(Game.isKeyDown('W') || Game.isKeyDown('A') || Game.isKeyDown('S') || Game.isKeyDown('D'))
        return true;
    if(jumpCheck && Game.isKeyDown(VK_SPACE))
        return true;
    return false;
}

void setbeed(float speed) {
    float yaw = Game.getLocalPlayer()->getActorRotationComponent()->mYaw; 
    Vec2<float> motion = getMotion(yaw, speed);                           

    if(!iszoving(false)) {
        motion.x = 0.f;
        motion.y = 0.f;
    }

    auto* stateVec = Game.getLocalPlayer()->getStateVectorComponent();
    stateVec->mVelocity.x = motion.x;  
    stateVec->mVelocity.z = motion.y;  
}


void ElytraFly::onNormalTick(LocalPlayer* actor) {
    LocalPlayer* localPlayer = Game.getLocalPlayer();
    float flySpeed = Speed;
    bool hasElytraInSlot =
        localPlayer->getArmor(1)->item &&
        localPlayer->getArmor(1)->item->mName.find("elytra") != std::string::npos;
    bool isRightSize = localPlayer->getAABBShapeComponent()->getAABB().size.y <= 0.6f;
    Vec3<float> currentPosPtr = Game.getLocalPlayer()->getPos();
    Vec3<float> currentPos = Vec3<float>(currentPosPtr.x, currentPosPtr.y, currentPosPtr.z);

    if(isRightSize && hasElytraInSlot) {
        setbeed(flySpeed);
        if(Game.isKeyDown(upKey)) {
            localPlayer->getStateVectorComponent()->mVelocity.y = Speedy / 10.0f;
        } else if(Game.isKeyDown(downKey)) {
            localPlayer->getStateVectorComponent()->mVelocity.y = -Speedy / 10.0f;
        } else {
            localPlayer->getStateVectorComponent()->mVelocity.y = 0.025f;
        }
        if(noclip) {
            AABB aabb = localPlayer->getAABBShapeComponent()->getAABB();
            aabb.upper.z = currentPos.z;
            aabb.upper.x = currentPos.x;
            aabb.lower.z = currentPos.z;
            aabb.lower.x = currentPos.x;
            aabb.upper.y = aabb.lower.y - 1.6f;
        }
        if(!noclip) {
            AABB aabb = localPlayer->getAABBShapeComponent()->getAABB();
            aabb.lower.x = currentPos.x - 0.35f;
            aabb.lower.z = currentPos.z - 0.35f;
            aabb.upper.x = currentPos.x + 0.35f;
            aabb.upper.z = currentPos.z + 0.35f;
        }
    }
}

void ElytraFly::onSendPacket(Packet* packet) {
    bool hasElytraInSlot =
        Game.getLocalPlayer()->getArmor(1)->item &&
        Game.getLocalPlayer()->getArmor(1)->item->mName.find("elytra") != std::string::npos;
    bool isRightSize = Game.getLocalPlayer()->getAABBShapeComponent()->getAABB().size.y <= 0.6f;
    if(isRightSize && hasElytraInSlot) {
        if(noclip) {
            static float offset = 0.0f;
            static bool goingUp = true;
            Vec3<float> currentPosPtr = Game.getLocalPlayer()->getPos();
            Vec3<float> currentPos1 =
                Vec3<float>(currentPosPtr.x, currentPosPtr.y, currentPosPtr.z);
            Vec3<float> currentPos = Game.getLocalPlayer()->getStateVectorComponent()->mPos;

            if(packet->getId() == PacketID::MovePlayer && Game.getLocalPlayer() != nullptr) {
                auto* movePacket = reinterpret_cast<MovePlayerPacket*>(packet);
                movePacket->mPos = currentPos1;
                movePacket->mPos = currentPos;
                movePacket->mOnGround = true;
            }
            if(packet->getId() == PacketID::PlayerAuthInput && Game.getLocalPlayer() != nullptr) {
                auto* authPacket = reinterpret_cast<PlayerAuthInputPacket*>(packet);
                authPacket->mPos = currentPos;
                authPacket->mPos = currentPos1;
                if(Game.isKeyDown(downKey)) {
                    authPacket->mAnalogMoveVector.y = -Speedy / 10.0f;
                }
            }
        }
        if(elytraspoofing) {
            static float verticalVelocity = 0.0f;
            static bool gliding = true;
            static float glideSpeed = 0.1f;
            Vec3<float> currentPosPtr = Game.getLocalPlayer()->getPos();
            Vec3<float> currentPos = Game.getLocalPlayer()->getStateVectorComponent()->mPos;

            if(packet->getId() == PacketID::MovePlayer && Game.getLocalPlayer() != nullptr) {
                auto* movePacket = reinterpret_cast<MovePlayerPacket*>(packet);
                verticalVelocity -= glideSpeed;
                currentPos.y += verticalVelocity;
                movePacket->mOnGround = false;
                movePacket->mPos = currentPos;
            }
        }
    }
}

void ElytraFly::onDisable() {
    if(noclip) {
        auto localPlayer = Game.getLocalPlayer();
        if(localPlayer != nullptr) {
            Vec3<float> currentPosPtr = localPlayer->getPos();
            Vec3<float> currentPos = Vec3<float>(currentPosPtr.x, currentPosPtr.y, currentPosPtr.z);

            AABB aabb = localPlayer->getAABBShapeComponent()->getAABB();
            aabb.upper.y = aabb.lower.y + 1.6f;
            aabb.lower.x = currentPos.x - 0.35f;
            aabb.lower.z = currentPos.z - 0.35f;
            aabb.upper.x = currentPos.x + 0.35f;
            aabb.upper.z = currentPos.z + 0.35f;
        }
    }
}
