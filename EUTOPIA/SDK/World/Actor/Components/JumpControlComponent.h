//
// Created by vastrakai on 7/10/2024.
//

#pragma once

#include "../../../../Utils/MemoryUtil.h"

struct JumpControlComponent {
public:
    // padding to make the struct size 88 bytes
    CLASS_MEMBER(bool, mNoJumpDelay, 0x10);

   char sb[0x58];
};

static_assert(sizeof(JumpControlComponent) == 0x58);