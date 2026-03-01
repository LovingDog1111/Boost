#pragma once

#include "ItemStack.h"
#include "../Level/Block/Block.h"

enum class SItemType { Helmet, Chestplate, Leggings, Boots, Sword, Pickaxe, Axe, Shovel, None };

class Block;
class Item {
public:
	CLASS_MEMBER(short, mItemId, 0xAA);
    CLASS_MEMBER(short, ItemId, 0xAA);  
	CLASS_MEMBER(int, ArmorItemType, 0x24C); // Offset for item name
	CLASS_MEMBER(int, mProtection, 0x26C); // Offset for item name
	CLASS_MEMBER(std::string, mName, 0xD8); // Offset for item name
	CLASS_MEMBER(uint16_t, maxStackSize, 0xA8); // Offset for item name
	CLASS_MEMBER(uint16_t, MaxUseDuration, 0x154); // Offset for item name

public:


	void setMaxDamage(int maxDamage) {
		MemoryUtil::callVirtualFunc<void>(25, this, maxDamage);
	}

	void setMaxUseDuration(int maxUseDuration) {
		MemoryUtil::callVirtualFunc<void>(26, this, maxUseDuration);
	}

	// Alternative implementation using header offset (0xA8)
	void setMaxStackSize(int maxStackSize) {
		*reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 0xA8) = maxStackSize;
	}

	float getMaxDamage() {
		return MemoryUtil::callVirtualFunc<float>(35, this);
	}

	bool isGlint(ItemStackBase* itemStackBase) {
		return MemoryUtil::callVirtualFunc<bool, ItemStackBase*>(39, this, itemStackBase);
	}

	short getDamageValue(CompoundTag* userData) {
		using func_t = short(__thiscall*)(Item*, CompoundTag*);
		static func_t Func = reinterpret_cast<func_t>(MemoryUtil::findSignature("40 53 48 83 EC ? 48 8B 51 ? 33 DB 48 85 D2"));
		return Func(this, userData);
	}
	
	void readCompoundTag(CompoundTag* compoundTag) {
		MemoryUtil::callVirtualFunc<void>(89, this, compoundTag);
	}
	void writeCompoundTag(CompoundTag* compoundTag) {
		MemoryUtil::callVirtualFunc<void>(90, this, compoundTag);
	}

	float getDestroySpeed(ItemStackBase* item, Block* block) {//test
		return MemoryUtil::callVirtualFunc<float, ItemStackBase*, Block*>(81, this, item, block);
	}
	float getDestroySpeed2(ItemStackBase* item,Block* block) {//test
		uintptr_t** mVfTable = reinterpret_cast<uintptr_t**>((uintptr_t)MemoryUtil::getVtableFromSig("48 8D 05 ? ? ? ? 48 89 05 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? C6 05 ? ? ? ? ? 48 8D 0D"));
		return MemoryUtil::callVirtualFunc<float, ItemStackBase*, uintptr_t**,	Block*>(81, this, item, mVfTable,block);
	}

int getArmorSlot() {
        // Use the name of the item to determine the armor slot
        static constexpr std::array<std::string_view, 4> armorSlots = {"_helmet", "_chestplate",
                                                                       "_leggings", "_boots"};

        for(int i = 0; i < armorSlots.size(); i++) {
            if(mName.find(armorSlots[i]) != std::string::npos) {
                return i;
            }
        }

        return -1;
    }

    bool isHelmet() {
        static constexpr std::string_view helmet = "_helmet";
        return mName.ends_with(helmet);
    }

    bool isChestplate() {
        static constexpr std::string_view chestplate = "_chestplate";
        return mName.ends_with(chestplate);
    }

    bool isLeggings() {
        static constexpr std::string_view leggings = "_leggings";
        return mName.ends_with(leggings);
    }

    bool isBoots() {
        static constexpr std::string_view boots = "_boots";
        return mName.ends_with(boots);
    }

    bool isSword() {
        static constexpr std::string_view sword = "_sword";
        return mName.ends_with(sword);
    }

    bool isPickaxe() {
        static constexpr std::string_view pickaxe = "_pickaxe";
        return mName.ends_with(pickaxe);
    }

    bool isAxe() {
        static constexpr std::string_view axe = "_axe";
        return mName.ends_with(axe);
    }

    bool isShovel() {
        static constexpr std::string_view shovel = "_shovel";
        return mName.ends_with(shovel);
    }

    int getItemTier() {
        // Use the name of the item to determine the tier
        static constexpr std::array<std::string_view, 7> tiers = {
            "wooden_", "chainmail_", "stone_", "iron_", "golden_", "diamond_", "netherite_"};

        for(int i = 0; i < tiers.size(); i++) {
            if(mName.starts_with(tiers[i])) {
                return i;
            }
        }

        return 0;
    }

    int getArmorTier() {
        // Use the name of the item to determine the tier
        static constexpr std::array<std::string_view, 7> tiers = {
            "leather_", "chainmail_", "iron_", "golden_", "diamond_", "netherite_"};

        for(int i = 0; i < tiers.size(); i++) {
            if(mName.starts_with(tiers[i])) {
                return i;
            }
        }

        return 0;
    }

    SItemType getItemType() {
        if(isHelmet())
            return SItemType::Helmet;
        if(isChestplate())
            return SItemType::Chestplate;
        if(isLeggings())
            return SItemType::Leggings;
        if(isBoots())
            return SItemType::Boots;
        if(isSword())
            return SItemType::Sword;
        if(isPickaxe())
            return SItemType::Pickaxe;
        if(isAxe())
            return SItemType::Axe;
        if(isShovel())
            return SItemType::Shovel;

        return SItemType::None;
    }



};