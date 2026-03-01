#pragma once
#pragma once
#include "../../SDK/GlobalInstance.h"

namespace PlayerUtil {
extern uint32_t selectedSlotServerSide;
bool canPlaceBlock(const BlockPos& blockPos, bool airPlace);
bool tryPlaceBlock(const BlockPos& blockPos, bool airPlace, bool packetPlace);
}  // namespace PlayerUtil
