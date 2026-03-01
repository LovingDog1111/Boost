//
// Created by vastrakai on 6/30/2024.
//

#pragma once

#include "../../../../Utils/MemoryUtil.h"
#include "../../../../Utils/Maths.h"

struct MoveInputComponent {
public:
    CLASS_MEMBER(bool, mIsMoveLocked, 0x8A);
    CLASS_MEMBER(bool, mIsSneakDown, 0x28);
    CLASS_MEMBER(bool, isSneaking, 0x28);
    CLASS_MEMBER(bool, isSneakDown, 0x28);
    CLASS_MEMBER(bool, mIsJumping, 0x2F);
    CLASS_MEMBER(bool, mIsJumping2, 0x80);
    CLASS_MEMBER(bool, mIsSprinting, 0x30);
    CLASS_MEMBER(bool, mForward, 0xD);
    CLASS_MEMBER(bool, mBackward, 0xE);
    CLASS_MEMBER(bool, mLeft, 0xF);
    CLASS_MEMBER(bool, mRight, 0x10);
    CLASS_MEMBER(Vec2<float>, mMoveVector, 0x48);

    // padding to make the struct size 136
    char sb[0x88];

    void setJumping(bool value) {
        reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(this) + 0x26)[0] = value;
        reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(this) + 0x80)[0] = value;
    }

    void setSneaking(bool value) {
        reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(this) + 0x28)[0] = value;
    }


    void reset(bool lockMove = false, bool resetMove = true) {
        mIsMoveLocked = lockMove;
        mIsSneakDown = false;
        mIsJumping = false;
        mIsJumping2 = false;
        mIsSprinting = false;
        if (resetMove)
        {
            mForward = false;
            mBackward = false;
            mLeft = false;
            mRight = false;
        }
        mMoveVector =Vec2<float>(0, 0);
    }
};

struct RawMoveInputComponent {
public:
    CLASS_MEMBER(bool, mIsMoveLocked, 0x82);
    CLASS_MEMBER(bool, mIsSneakDown, 0x20);
    CLASS_MEMBER(bool, mIsJumping, 0x26);
    CLASS_MEMBER(bool, mIsJumping2, 0x80);
    CLASS_MEMBER(bool, mIsSprinting, 0x27);
    CLASS_MEMBER(bool, mForward, 0x2C);
    CLASS_MEMBER(bool, mBackward, 0x2D);
    CLASS_MEMBER(bool, mLeft, 0x2E);
    CLASS_MEMBER(bool, mRight, 0x2F);
    CLASS_MEMBER(Vec2<float>, mMoveVector, 0x48);

    // padding to make the struct size 136
    char pad_0x0[0x88];

    void setJumping(bool value) {
        reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(this) + 0x26)[0] = value;
        reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(this) + 0x80)[0] = value;
    }
    void setSneaking(bool value) {
        reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(this) + 0x20)[0] = value;
    }

};

static_assert(sizeof(MoveInputComponent) == 136, "MoveInputComponent size is not 136 bytes!");
static_assert(sizeof(RawMoveInputComponent) == 136, "RawMoveInputComponent size is not 136 bytes!");