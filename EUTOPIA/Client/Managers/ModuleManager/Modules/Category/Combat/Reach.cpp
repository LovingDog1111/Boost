#include "Reach.h"

Reach::Reach() : Module("Reach", "Extend reach.", Category::COMBAT) {
    registerSetting(new SliderSetting<float>("Reach", "NULL", &reachValue, 7.f, 3.f, 10.f));
}

void Reach::onEnable() {
    if(SurvivalreachPtr != nullptr) {
        *SurvivalreachPtr = 3.0f;
        DWORD oldProt = 0;
        VirtualProtect(SurvivalreachPtr, sizeof(float), oldProt, &oldProt);
    }
}

void Reach::onNormalTick(LocalPlayer* localPlayer) {
    if(SurvivalreachPtr == nullptr) {
        uintptr_t survivalReachSig =
            MemoryUtil::findSignature("F3 0F 10 05 ? ? ? ? 44 0F 2F F8");
        uintptr_t survivalReachOffsetRef = survivalReachSig + 4;
        uint32_t survivalReachOffset = *reinterpret_cast<uint32_t*>(survivalReachOffsetRef);
        SurvivalreachPtr =
            reinterpret_cast<float*>(survivalReachOffsetRef + survivalReachOffset + 4);

        DWORD oldProt = 0;
        VirtualProtect(SurvivalreachPtr, 4, PAGE_EXECUTE_READWRITE, &oldProt);
    }

    if(SurvivalreachPtr != nullptr) {
        *SurvivalreachPtr = reachValue;
    }
}
