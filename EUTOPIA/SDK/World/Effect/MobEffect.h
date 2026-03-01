#pragma once
#include "../../../Utils/MemoryUtil.h"
#include "../../Core/mce.h"




class MobEffect {
public:

    enum class EffectType : unsigned int {
        Empty = 0,
        Speed = 1,
        Slowness = 2,
        Haste = 3,
        MiningFatigue = 4,
        Strength = 5,
        InstantHealth = 6,
        InstantDamage = 7,
        JumpBoost = 8,
        Nausea = 9,
        Regeneration = 10,
        Resistance = 11,
        FireResistance = 12,
        WaterBreathing = 13,
        Invisibility = 14,
        Blindness = 15,
        NightVision = 16,
        Hunger = 17,
        Weakness = 18,
        Poison = 19,
        Wither = 20,
        HealthBoost = 21,
        Absorption = 22,
        Saturation = 23,
        Levitation = 24,
        FatalPoison = 25,
        ConduitPower = 26,
        SlowFalling = 27,
        BadOmen = 28,
        VillageHero = 29,
        Darkness = 30,
    };
    struct FactorCalculationData {
    public:
        FactorCalculationData() = delete;

        int                                              mPaddingDuration;        // this+0x0
        float                                            mFactorStart;            // this+0x4
        float                                            mFactorTarget;           // this+0x8
        float                                            mFactorCurrent;          // this+0xC
        std::function<void(FactorCalculationData&, int)> mUpdateFn;               // this+0x10
        int                                              mEffectChangedTimestamp; // this+0x50
        float                                            mFactorPreviousFrame;    // this+0x54
        bool                                             mHadEffectLastTick;      // this+0x58
        bool                                             mHadApplied;             // this+0x59
    };

public:
	CLASS_MEMBER(mce::Color, color, 0x10);
public:
	static MobEffect* getById(uint32_t id) {
		if (id > 0x24)
			return nullptr;

		static MobEffect** effectList = (MobEffect**)(uintptr_t)(MemoryUtil::getVtableFromSig(
            "48 8D 0D ? ? ? ? ? ? ? ? 48 85 C0 74 ? 0F B6 80 ? ? ? ? 24 ? 88 45 ? 0F B7 57"));
		return effectList[id];
	}
};