#pragma once
#include "../../ModuleBase/Module.h"
class Criticals : public Module {
   private:
    int mode = 0;

   public:
    Criticals();

    bool jumping = true;

    bool mVelocity = true;
    bool mPositionChange = true;
    bool mBiggerPositionChange = true;
    bool mSendJumping = true;
    bool mOffSprint = true;
    float mPositionChangePersent = 1.5f;

    enum class AnimationState { START, MID_AIR, MID_AIR2, LANDING, FINISHED };
    AnimationState mAnimationState = AnimationState::START;

    float mJumpPositions[4] = {0, 0.8200100660324097 - 0.6200100183486938,
                               0.741610050201416 - 0.6200100183486938, 0};
    float mJumpPositionsMini[4] = {0, 0.02, 0.01, 0};
    float mJumpVelocities[4] = {-0.07840000092983246, -0.07840000092983246, -0.1552319973707199f,
                                -0.07840000092983246};

    std::string getModeText() override;
    void onSendPacket(Packet* packet) override;
};