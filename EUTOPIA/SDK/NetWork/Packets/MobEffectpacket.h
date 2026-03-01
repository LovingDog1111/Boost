#pragma once

#include "Packet.h"

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

class MobEffectPacket : public ::Packet {
   public:
    static const PacketID ID = PacketID::MobEffect;

    // MobEffectPacket inner types define
    enum class Event : signed char {
        Invalid = 0x0,
        Add = 0x1,
        Update = 0x2,
        Remove = 0x3,
    };

    uint64_t mRuntimeId;        // this+0x30
    int mEffectDurationTicks;   // this+0x38
    enum class Event mEventId;  // this+0x3C
    EffectType mEffectId;       // this+0x40
    int mEffectAmplifier;       // this+0x44
    bool mShowParticles;        // this+0x48
    uint64_t mTick;             // this+0x50

    std::string_view getEffectName() {
        static const char* effectNames[] = {
            "Empty",           // 0
            "Speed",           // 1
            "Slowness",        // 2
            "Haste",           // 3
            "MiningFatigue",   // 4
            "Strength",        // 5
            "InstantHealth",   // 6
            "InstantDamage",   // 7
            "JumpBoost",       // 8
            "Nausea",          // 9
            "Regeneration",    // 10
            "Resistance",      // 11
            "FireResistance",  // 12
            "WaterBreathing",  // 13
            "Invisibility",    // 14
            "Blindness",       // 15
            "NightVision",     // 16
            "Hunger",          // 17
            "Weakness",        // 18
            "Poison",          // 19
            "Wither",          // 20
            "HealthBoost",     // 21
            "Absorption",      // 22
            "Saturation",      // 23
            "Levitation",      // 24
            "FatalPoison",     // 25
            "ConduitPower",    // 26
            "SlowFalling",     // 27
            "BadOmen",         // 28
            "VillageHero",     // 29
            "Darkness"         // 30
        };
        
        unsigned int index = static_cast<unsigned int>(mEffectId);
        if (index < sizeof(effectNames) / sizeof(effectNames[0])) {
            return effectNames[index];
        }
        return "Unknown";
    }

    std::string_view getEventName() {
        static const char* eventNames[] = {
            "Invalid",  // 0
            "Add",      // 1
            "Update",   // 2
            "Remove"    // 3
        };
        
        signed char index = static_cast<signed char>(mEventId);
        if (index >= 0 && index < sizeof(eventNames) / sizeof(eventNames[0])) {
            return eventNames[index];
        }
        return "Unknown";
    }
};