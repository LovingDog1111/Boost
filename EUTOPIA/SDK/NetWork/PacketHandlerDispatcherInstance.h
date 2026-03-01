#pragma once

#include <memory>
#include "../Utils/MemoryUtil.h"

class PacketHandlerDispatcherInstance {
   public:
    CLASS_MEMBER(uintptr_t**, vtable, 0x0);


   public:
    uintptr_t getPacketHandler() {
        return MemoryUtil::GetVTableFunction(this, 1);
    }
};