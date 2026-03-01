#pragma once
#include "../../ModuleBase/Module.h"

class CritsCC : public Module {
   public:
    enum class AnimationState { START, MID_AIR, MID_AIR2, LANDING, FINISHED };

    bool velocity = true;
    bool positionChange = true;
    bool biggerPositionChange = true;
    bool sendJumping = true;
    bool offSprint = true;
    float positionPercent = 1.5f;

    bool mWasSprinting = true;
    AnimationState mAnimationState = AnimationState::START;

    float mJumpPositions[4] = {0, 0.8200100660324097f - 0.6200100183486938f,
                               0.741610050201416f - 0.6200100183486938f, 0};
    float mJumpPositionsMini[4] = {0, 0.02f, 0.01f, 0};
    float mJumpVelocities[4] = {-0.07840000092983246f, -0.07840000092983246f, -0.1552319973707199f,
                                -0.07840000092983246f};

    CritsCC();
    void onSendPacket(Packet* packet) override;
};
