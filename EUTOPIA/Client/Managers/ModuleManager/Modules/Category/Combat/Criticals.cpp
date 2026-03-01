#include "Criticals.h"

Criticals::Criticals() : Module("Criticals", "Each hit is a critical hit", Category::COMBAT) {
    registerSetting(new EnumSetting("Mode", "NULL", {"Nukkit", "Hive"}, &mode, 0));
    registerSetting(new BoolSetting("Send Jumping", "Send jumping packet to server", &jumping, true));
    
    // Hive 模式的高级设置
    registerSetting(new BoolSetting("Use Velocity", "Whether to use velocity change", &mVelocity, true));
    registerSetting(new BoolSetting("Position Change", "Whether to change pos", &mPositionChange, true));
    registerSetting(new BoolSetting("Bigger Position Change", "Whether to change more pos", &mBiggerPositionChange, true));
    registerSetting(new BoolSetting("Send Jumping", "Whether to send jumping", &mSendJumping, true));
    registerSetting(new BoolSetting("Disable Sprint", "Whether to disable sprint", &mOffSprint, true));
    registerSetting(new SliderSetting<float>("Position Change Percent", "Changes delta", &mPositionChangePersent, 1.5f, 0.0f, 2.0f));
}

std::string Criticals::getModeText() {
    switch(mode) {
        case 0: {
            return "Nukkit";
            break;
        }
        case 1: {
            return "Hive";
            break;
        }
    }
    return "NULL";
}

void Criticals::onSendPacket(Packet* packet) {
    static float offsetY = 0.0f;

    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(localPlayer == nullptr) {
        offsetY = 0.0f;
        return;
    }

    if(packet->getId() == PacketID::PlayerAuthInput) {
        PlayerAuthInputPacket* paip = static_cast<PlayerAuthInputPacket*>(packet);

        if(mode == 0) { // Nukkit 模式
            paip->mPos.y -= offsetY;
            offsetY += 0.012f;
            if(offsetY >= 0.24f) {
                offsetY = 0.0f;
            }
        }

        if(mode == 1) { // Hive 模式 
            auto mStateVector = localPlayer->getStateVectorComponent();
            if(mStateVector->mPosOld.y == mStateVector->mPos.y) {
                
                if(mSendJumping) {
                    paip->mInputData |= AuthInputAction::JUMP_DOWN;
                    paip->mInputData |= AuthInputAction::JUMPING;
                    paip->mInputData |= AuthInputAction::WANT_UP;
                }

                switch(mAnimationState) {
                    case AnimationState::START:
                        if(mVelocity)
                            paip->mPosDelta.y = -0.07840000092983246f;
                        mAnimationState = AnimationState::MID_AIR;
                        break;

                    case AnimationState::MID_AIR:
                        if(mPositionChange)
                            paip->mPosDelta.y += mBiggerPositionChange ? mJumpPositions[1] * mPositionChangePersent : mJumpPositionsMini[1] * mPositionChangePersent;
                        if(mVelocity)
                            paip->mPosDelta.y = -0.07840000092983246f;
                        mAnimationState = AnimationState::MID_AIR2;
                        break;

                    case AnimationState::MID_AIR2:
                        if(mSendJumping) {
                            paip->mInputData &= ~AuthInputAction::JUMP_DOWN;
                            paip->mInputData &= ~AuthInputAction::JUMPING;
                            paip->mInputData &= ~AuthInputAction::WANT_UP;
                        }

                        if(mPositionChange)
                            paip->mPosDelta.y += mBiggerPositionChange ? mJumpPositions[2] * mPositionChangePersent : mJumpPositionsMini[2] * mPositionChangePersent;
                        if(mVelocity)
                            paip->mPosDelta.y = -0.1552319973707199f;
                        mAnimationState = AnimationState::LANDING;
                        break;

                    case AnimationState::LANDING:
                        if(mVelocity)
                            paip->mPosDelta.y = -0.07840000092983246f;
                        mAnimationState = AnimationState::FINISHED;
                        break;

                    case AnimationState::FINISHED:
                        mAnimationState = AnimationState::START;
                        break;
                }

                if(mOffSprint) {
                    paip->mInputData &= ~AuthInputAction::START_SPRINTING;
                    paip->mInputData &= ~AuthInputAction::SPRINTING;
                    paip->mInputData &= ~AuthInputAction::SPRINT_DOWN;
                    paip->mInputData &= ~AuthInputAction::STOP_SPRINTING;
                }
            }
        }
    }
}