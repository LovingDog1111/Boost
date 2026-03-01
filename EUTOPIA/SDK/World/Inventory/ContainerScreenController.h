#pragma once
#include <string>

class ContainerScreenController {
   public:
    void handleAutoPlace(const std::string& name, int slot) {
        static auto func = MemoryUtil::getFuncFromCall(
            MemoryUtil::findSignature("E8 ? ? ? ? 66 ? ? ? ? ? ? ? 0F 8C"));
        return MemoryUtil::callFastcall<void>(func, this, 0x7FFFFFFF, name, slot);
    }

    void* _tryExit() {
        // TODO: implement index signature instead of raw index
        return MemoryUtil::callVirtualFunc<void*>(12, this);
    }
};


