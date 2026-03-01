#pragma once

#include "../../../Utils/MemoryUtil.h"
#include "ItemStackBase.h"
enum class Enchant : int {
    PROTECTION = 0,
    FIRE_PROTECTION = 1,
    FEATHER_FALLING = 2,
    BLAST_PROTECTION = 3,
    PROJECTILE_PROTECTION = 4,
    THORNS = 5,
    RESPIRATION = 6,
    DEPTH_STRIDER = 7,
    AQUA_AFFINITY = 8,
    SHARPNESS = 9,
    SMITE = 10,
    BANE_OF_ARTHROPODS = 11,
    KNOCKBACK = 12,
    FIRE_ASPECT = 13,
    LOOTING = 14,
    EFFICIENCY = 15,
    SILK_TOUCH = 16,
    UNBREAKING = 17,
    FORTUNE = 18,
    POWER = 19,
    PUNCH = 20,
    FLAME = 21,
    BOW_INFINITY = 22,
    LUCK_OF_THE_SEA = 23,
    LURE = 24,
    FROST_WALKER = 25,
    MENDING = 26,
    BINDING = 27,
    VANISHING = 28,
    IMPALING = 29,
    RIPTIDE = 30,
    LOYALTY = 31,
    CHANNELING = 32,
    MULTISHOT = 33,
    PIERCING = 34,
    QUICK_CHARGE = 35,
    SOUL_SPEED = 36,
    SWIFT_SNEAK = 37
};

class ItemStack : public ItemStackBase {
   public:
    uint8_t mStackNetId;
    char sb[0x8];

    // Added members
    Item* item = nullptr;
    int mCount = 0;

    // Added method
    bool isValid() const {
        return item != nullptr && mCount > 0;
    }

    void reinit(Item* item, int count, int itemData) {
        mVfTable = reinterpret_cast<uintptr_t**>((uintptr_t)MemoryUtil::getVtableFromSig(
            "48 8D 05 ? ? ? ? 48 89 85 ? ? ? ? 44 89 A5 ? ? ? ? C6 85 ? ? ? ? ? 0F B6 05"));
        MemoryUtil::callVirtualFunc<void>(3, this, item, count, itemData);

        // Store for access later
        this->item = item;
        this->mCount = count;
    }
    // 48 8D 35 ? ? ? ? 48 8B 87
    static ItemStack* getEmptyItem() {
        return reinterpret_cast<ItemStack*>(
            (uintptr_t)MemoryUtil::getVtableFromSig("48 8D 35 ? ? ? ? 48 8B 87"));
    }

    // MemoryUtil::getFuncFromCall(MemoryUtil::findSignature("48 89 5C 24 10 48 89 7C 24 18 55 48 8D
    // AC 24 30 FF FF FF 48 81 EC D0 01 00 00 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 85 C0 00 00 00 48
    // 8B D9 E8") float getDestroySpeed(Block* block) {
    //     using func_t = float(__thiscall*)(ItemStack*, Block*);
    //     static func_t Func =
    //     reinterpret_cast<func_t>(MemoryUtil::getFuncFromCall(MemoryUtil::findSignature("48 89 5C
    //     24 10 48 89 7C 24 18 55 48 8D AC 24 30 FF FF FF 48 81 EC D0 01 00 00 48 8B 05 ?? ?? ?? ??
    //     48 33 C4 48 89 85 C0 00 00 00 48 8B D9 E8"))); return Func(this, block);
    // }

    std::string getCustomName() {
        std::string str;
        static auto address = MemoryUtil::findSignature(
            "48 89 ? ? ? 57 48 83 EC ? 48 8B ? 48 8B ? E8 ? ? ? ? 84 C0 74 ? 48 8B ? 48 8B ? E8 ? "
            "? ? ? 48 8B");
        MemoryUtil::callFastcall<void, ItemStack*, std::string*>(address, this, &str);
        return str;
    }

    std::map<int, int> gatherEnchants() {
        if(!mCompoundTag)
            return {};

        std::map<int, int> enchants;
        for(auto& [first, second] : mCompoundTag->data) {
            if(second.type != Tag::Type::List || first != "ench")
                continue;

            for(const auto list = second.asListTag(); const auto& entry : list->val) {
                if(entry->getId() != Tag::Type::Compound)
                    continue;

                const auto comp = reinterpret_cast<CompoundTag*>(entry);
                int id = comp->get("id")->asShortTag()->val;
                const int lvl = comp->get("lvl")->asShortTag()->val;
                enchants[id] = lvl;
            }
        }

        return enchants;
    }

    int getEnchantValue(int id) {
        auto enchants = gatherEnchants();
        return enchants.contains(id) ? enchants[id] : 0;
    }

    int getEnchantValue(Enchant enchant) {
        return getEnchantValue(static_cast<int>(enchant));
    }

    ItemStack() {
        memset(this, 0, sizeof(ItemStack));
        mVfTable = reinterpret_cast<uintptr_t**>((uintptr_t)MemoryUtil::getVtableFromSig(
            "48 8D 05 ? ? ? ? 48 89 85 ? ? ? ? 44 89 A5 ? ? ? ? C6 85 ? ? ? ? ? 0F B6 05"));

        // Initialize added members explicitly in constructor
        item = nullptr;
        mCount = 0;
    }

    


    ItemStack(Item* item, int count, int itemData) {
        memset(this, 0x0, sizeof(ItemStack));
        reinit(item, count, itemData);
    }

    std::string getEnchantName(Enchant enchant) const {
        static const char* names[] = {"PROTECTION",
                                      "FIRE_PROTECTION",
                                      "FEATHER_FALLING",
                                      "BLAST_PROTECTION",
                                      "PROJECTILE_PROTECTION",
                                      "THORNS",
                                      "RESPIRATION",
                                      "DEPTH_STRIDER",
                                      "AQUA_AFFINITY",
                                      "SHARPNESS",
                                      "SMITE",
                                      "BANE_OF_ARTHROPODS",
                                      "KNOCKBACK",
                                      "FIRE_ASPECT",
                                      "LOOTING",
                                      "EFFICIENCY",
                                      "SILK_TOUCH",
                                      "UNBREAKING",
                                      "FORTUNE",
                                      "POWER",
                                      "PUNCH",
                                      "FLAME",
                                      "BOW_INFINITY",
                                      "LUCK_OF_THE_SEA",
                                      "LURE",
                                      "FROST_WALKER",
                                      "MENDING",
                                      "BINDING",
                                      "VANISHING",
                                      "IMPALING",
                                      "RIPTIDE",
                                      "LOYALTY",
                                      "CHANNELING",
                                      "MULTISHOT",
                                      "PIERCING",
                                      "QUICK_CHARGE",
                                      "SOUL_SPEED",
                                      "SWIFT_SNEAK"};

        std::string name =
            (static_cast<int>(enchant) < 38) ? names[static_cast<int>(enchant)] : "UNKNOWN";
        std::transform(name.begin(), name.end(), name.begin(),
                       [](char c) { return c == '_' ? ' ' : std::tolower(c); });
        return name;
    }
};
