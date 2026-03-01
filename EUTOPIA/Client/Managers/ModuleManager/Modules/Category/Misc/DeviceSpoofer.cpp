#include "DeviceSpoofer.h"

#include "..\Client\Managers\HooksManager\HookManager.h"



DeviceSpoofer::DeviceSpoofer() : Module("DeviceSpoofer", "Spoof your device id", Category::MISC) {}

void DeviceSpoofer::onEnable() {
    uintptr_t addr = MemoryUtil::findSignature("48 89 5C 24 18 48 89 74 24 20 57 48 83 EC 60 48 8B FA 48 8B D9 48 8B");

    if(addr) {
        char msg[128];
        snprintf(msg, sizeof(msg), "[DeviceSpoofer] Hooked at: %p", (void*)addr);
        GI::DisplayClientMessage(msg);


    } else {
        GI::DisplayClientMessage(
            "[DeviceSpoofer] Failed to find signature");
    }
}

void DeviceSpoofer::onDisable() {;
    GI::DisplayClientMessage("[DeviceSpoofer] Disabled");
}
