#include "CritsCC.h"



CritsCC::CritsCC() : Module("CritsCC", "Each hit is a critical hit (cubecraft)", Category::COMBAT) {
    registerSetting(new BoolSetting("Velocity", "", &velocity, true));
    registerSetting(new BoolSetting("PositionChange", "", &positionChange, true));
    registerSetting(new BoolSetting("BiggerPositionChange", "", &biggerPositionChange, true));
    registerSetting(new BoolSetting("SendJumping", "", &sendJumping, true));
    registerSetting(new BoolSetting("DisableSprint", "", &offSprint, true));
    registerSetting(new SliderSetting<float>("PositionPercent", "", &positionPercent, 1.5f, 0.f, 2.f));
}

void CritsCC::onSendPacket(Packet* packet) {
    auto player = GI::getClientInstance();
    if(!player)
        return;

    if(packet->getId() == PacketID::PlayerAuthInput) {
        auto paip = reinterpret_cast<PlayerAuthInputPacket*>(packet);
        auto unitz = GI::getLocalPlayer();
        auto state = unitz->getStateVectorComponent(); // sexyyyyy

        if(state->mPosOld.y == state->mPos.y) {
            if(sendJumping) {
                paip->mInputData |= AuthInputAction::JUMP_DOWN;
                paip->mInputData |= AuthInputAction::JUMPING;
                paip->mInputData |= AuthInputAction::WANT_UP;
            }

            switch(mAnimationState) {
                case AnimationState::START:
                    if(velocity)
                        paip->mPosDelta.y = mJumpVelocities[0];
                    mAnimationState = AnimationState::MID_AIR;
                    break;

                case AnimationState::MID_AIR:
                    if(positionChange)
                        paip->mPosDelta.y +=
                            (biggerPositionChange ? mJumpPositions[1] : mJumpPositionsMini[1]) *
                            positionPercent;
                    if(velocity)
                        paip->mPosDelta.y = mJumpVelocities[1];
                    mAnimationState = AnimationState::MID_AIR2;
                    break;

                case AnimationState::MID_AIR2:
                    if(sendJumping) {
                        paip->mInputData &= ~AuthInputAction::JUMP_DOWN;
                        paip->mInputData &= ~AuthInputAction::JUMPING;
                        paip->mInputData &= ~AuthInputAction::WANT_UP;
                    }
                    if(positionChange)
                        paip->mPosDelta.y +=
                            (biggerPositionChange ? mJumpPositions[2] : mJumpPositionsMini[2]) *
                            positionPercent;
                    if(velocity)
                        paip->mPosDelta.y = mJumpVelocities[2];
                    mAnimationState = AnimationState::LANDING;
                    break;

                case AnimationState::LANDING:
                    if(velocity)
                        paip->mPosDelta.y = mJumpVelocities[3];
                    mAnimationState = AnimationState::FINISHED;
                    break;

                case AnimationState::FINISHED:
                    mAnimationState = AnimationState::START;
                    break;
            }
        }
    }

    if(offSprint) {
        auto paip = reinterpret_cast<PlayerAuthInputPacket*>(packet);
        paip->mInputData &= ~AuthInputAction::START_SPRINTING;
        paip->mInputData &= ~AuthInputAction::SPRINTING;
        paip->mInputData &= ~AuthInputAction::SPRINT_DOWN;
        paip->mInputData &= ~AuthInputAction::STOP_SPRINTING;
    }
}
