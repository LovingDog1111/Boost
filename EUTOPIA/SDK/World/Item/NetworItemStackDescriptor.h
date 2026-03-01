#pragma once
#include "../../Utils/MemoryUtil.h"


class NetworkItemStackDescriptor
{
public:
    char sb[0x60];
    /*bool                  mIncludeNetIds{};  // this+0x18
    ItemStackNetIdVariant mNetIdVariant{};   // this+0x20
    unsigned int          mBlockRuntimeId{}; // this+0x38
    std::string           mUserDataBuffer{}; // this+0x40*/

    NetworkItemStackDescriptor() = default;
    NetworkItemStackDescriptor(class ItemStack const& itemStack) {
        memset(this, 0, sizeof(NetworkItemStackDescriptor));

        static uintptr_t funcAddr = 0;
        if(funcAddr == 0) {
            uintptr_t callAddr = MemoryUtil::findSignature(
                "E8 ? ? ? ? 90 48 8B ? 48 8D ? ? ? ? ? E8 ? ? ? ? 4C 8D ? ? ? ? ? 4C 89 ? ? ? ? ? "
                "48 8D");
            funcAddr = MemoryUtil::getFuncFromCall(callAddr);
        }

        using FuncType = void(__fastcall*)(NetworkItemStackDescriptor*, const ItemStack*);
        auto func = reinterpret_cast<FuncType>(funcAddr);
        func(this, &itemStack);
    }
};

static_assert(sizeof(NetworkItemStackDescriptor) == 0x60);