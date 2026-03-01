#include "AutoArmor.h"

#include <chrono>

#include "../Utils/Minecraft/ItemUtils.h"

static uint64_t nowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

// equip armor crashes :(

AutoArmor::AutoArmor() : Module("AutoArmor", "Equips best armor automatically", Category::PLAYER) {}

void AutoArmor::onNormalTick(LocalPlayer* lp) {
    if(!lp)
        return;
    auto supplies = lp->getsupplies();
    if(!supplies)
        return;
    auto container = supplies->getcontainer();
    if(!container)
        return;
    auto armor = lp->getArmorContainer();
    if(!armor)
        return;

    if(lastAction + delay > nowMs())
        return;

    int currentHelmet = ItemUtils::getItemValue(armor->getItem(0));
    int currentChest = ItemUtils::getItemValue(armor->getItem(1));
    int currentLegs = ItemUtils::getItemValue(armor->getItem(2));
    int currentBoots = ItemUtils::getItemValue(armor->getItem(3));

    int bestHelmet = -1, bestChest = -1, bestLegs = -1, bestBoots = -1;
    int bestHelmetVal = currentHelmet, bestChestVal = currentChest, bestLegsVal = currentLegs,
        bestBootsVal = currentBoots;

    for(int i = 0; i < 36; i++) {
        auto item = container->getItem(i);
        if(!item->mItem)
            continue;

        int v = ItemUtils::getItemValue(item);
        auto t = item->getItem()->getItemType();

        if(t == SItemType::Helmet && v > bestHelmetVal) {
            bestHelmet = i;
            bestHelmetVal = v;
        } else if(t == SItemType::Chestplate && v > bestChestVal) {
            bestChest = i;
            bestChestVal = v;
        } else if(t == SItemType::Leggings && v > bestLegsVal) {
            bestLegs = i;
            bestLegsVal = v;
        } else if(t == SItemType::Boots && v > bestBootsVal) {
            bestBoots = i;
            bestBootsVal = v;
        }
    }

    if(bestHelmet != -1) {
        supplies->equipArmor(bestHelmet);
        lastAction = nowMs();
        return;
    }
    if(bestChest != -1) {
        supplies->equipArmor(bestChest);
        lastAction = nowMs();
        return;
    }
    if(bestLegs != -1) {
        supplies->equipArmor(bestLegs);
        lastAction = nowMs();
        return;
    }
    if(bestBoots != -1) {
        supplies->equipArmor(bestBoots);
        lastAction = nowMs();
        return;
    }
}
