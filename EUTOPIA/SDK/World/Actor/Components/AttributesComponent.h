//
// Created by vastrakai on 7/12/2024.
//

#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "../../../../Utils/MemoryUtil.h"

enum AttributeId {
    Health = 7,
    Absorption = 14,
};

class AttributeInstance {
public:
    char pad1[0x60];

    union {
        float mDefaultValues[3];
        struct {
            float mDefaultMinValue;
            float mDefaultMaxValue;
            float mDefaultValue;
        };
    };

    union {
        float mCurrentValues[3];
        struct {
            float mCurrentMinValue;
            float mCurrentMaxValue;
            float mCurrentValue;
        };
    };

    virtual ~AttributeInstance();
    virtual void tick();
};

static_assert(sizeof(AttributeInstance) == 0x80, "AttributeInstance size is not correct");

class Attribute {
public: // I'm pretty sure this is a HashedString but i don't really care
    __int64 mHash;
    __int64 mHashedStringHash;
    std::string mAttributeName;
    char sb [0x32];

    Attribute() {
        memset(this, 0x0, sizeof(Attribute));
    }

    Attribute(__int64 hash) {
        memset(this, 0x0, sizeof(Attribute));
        this->mHash = hash;
    }
};

enum AttributeHashes : unsigned __int64 {
    HEALTH = 30064771328 - 4294967296,
    HUNGER = 8589934848 - 4294967296,
    MOVEMENT = 42949673217 - 4294967296,
    ABSORPTION = 60129542401 - 4294967296,
    SATURATION = 12884902144 - 4294967296,
    FOLLOW_RANGE = 34359738369 - 4294967296,
    LEVEL = 21474836736 - 4294967296,
    EXPERIENCE = 25769804032 - 4294967296
};

class HealthAttribute : public Attribute
{
public:
    HealthAttribute() { this->mHash = AttributeHashes::HEALTH; }
};

class PlayerHungerAttribute : public Attribute
{
public:
    PlayerHungerAttribute() { this->mHash = AttributeHashes::HUNGER; }
};

class MovementAttribute : public Attribute
{
public:
    MovementAttribute() { this->mHash = AttributeHashes::MOVEMENT; }
};

class AbsorptionAttribute : public Attribute
{
public:
    AbsorptionAttribute() { this->mHash = AttributeHashes::ABSORPTION; }
};

class PlayerSaturationAttribute : public Attribute
{
public:
    PlayerSaturationAttribute() { this->mHash = AttributeHashes::SATURATION; }
};

class FollowRangeAttribute : public Attribute
{
public:
    FollowRangeAttribute() { this->mHash = AttributeHashes::FOLLOW_RANGE; }
};

class PlayerLevelAttribute : public Attribute {
public:
    PlayerLevelAttribute() { this->mHash = AttributeHashes::LEVEL; }
};

class PlayerExperienceAttribute : public Attribute
{
public:
    PlayerExperienceAttribute() { this->mHash = AttributeHashes::EXPERIENCE; }
};




class BaseAttributeMap
{
public:
    std::unordered_map<int, AttributeInstance> mAttributes;
    //std::vector<uint64_t> mDirtyAttributes;
private:
    char sb2[0x20];
public:

    AttributeInstance* getInstance(unsigned int id)
    {
     static auto addr = MemoryUtil::findSignature(
         "4C 8B C9 89 54 24 ? 48 B9 ? ? ? ? ? ? ? ? 44 0F B6 C2 48 B8 ? ? ? ? ? ? ? ? 4C 33 C0 0F "
         "B6 44 24 ? 4C 0F AF C1 4C 33 C0 0F B6 44 24 ? 4C 0F AF C1 4C 33 C0 0F B6 44 24 ? 4C 0F "
         "AF C1 4C 33 C0 4C 0F AF C1 49 8B 49 ? 49 23 C8 4D 8B 41 ? 48 C1 E1 ? 49 03 49 ? 48 8B 41 "
         "? 49 3B C0 74 ? 48 8B 09 3B 50 ? 74 ? 66 90 48 3B C1 74 ? 48 8B 40 ? 3B 50 ? 75 ? EB ? "
         "33 C0 48 85 C0 48 8D 15 ? ? ? ? 49 0F 44 C0 49 3B C0 48 8D 48 ? 48 0F 45 D1 48 8B C2 C3 "
         "CC 48 89 5C 24");
        return MemoryUtil::callFastcall<AttributeInstance*>(addr, this, id);
    }
};

static_assert(sizeof(BaseAttributeMap) == 0x60);


struct AttributesComponent
{
    BaseAttributeMap mBaseAttributeMap;
};

static_assert(sizeof(AttributesComponent) == 0x60);