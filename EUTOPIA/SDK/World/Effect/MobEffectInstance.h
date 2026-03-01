#pragma once
#include <cstdint>
#include <string>
#include "MobEffect.h"
class MobEffectInstance {
public:
    MobEffectInstance();

    MobEffect::EffectType            mId;                              // this+0x0
    int                              mDuration;                        // this+0x4
    int                              mDurationPeaceful;                // this+0x8
    int                              mDurationEasy;                    // this+0xC
    int                              mDurationNormal;                  // this+0x10
    int                              mDurationHard;                    // this+0x14
    int                              mAmplifier;                       // this+0x18
    char                             pad_0008[0x8];                    // this+0x1C
    bool                             mDisplayOnScreenTextureAnimation; // this+0x24
    bool                             mAmbient;                         // this+0x25
    bool                             mNoCounter;                       // this+0x26
    bool                             mEffectVisible;                   // this+0x27
    MobEffect::FactorCalculationData mFactorCalculationData;           // this+0x28 
public:
    std::string getDisplayName() {
        // Search "potion.potency." and compare to server database!
        using func_t = std::string* (__thiscall*)(MobEffectInstance*, std::string&);
        static func_t Func = (func_t)MemoryUtil::findSignature("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 ? 48 8B F2 4C 8B E1 48 89 54 24");//48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 ? 48 8B F2 4C 8B E1 48 89 54 24
        std::string ncct;
        return *Func(this, ncct);
    }
};

static_assert(sizeof(MobEffectInstance) == 0x88);