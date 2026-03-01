#include "ItemUtils.h"

int ItemUtils::getItemValue(ItemStack* item) {
    if(!item->mItem)
        return -1;

    int value = 0;
    auto type = item->getItem()->getItemType();

    switch(type) {
        case SItemType::Helmet:
        case SItemType::Chestplate:
        case SItemType::Leggings:
        case SItemType::Boots:
            if(item->getItem()->getItemTier() < 4)
                break;
            value = item->getEnchantValue(Enchant::PROTECTION) +
                    item->getEnchantValue(Enchant::FIRE_PROTECTION);
            value += item->getItem()->getArmorTier();
            break;

        case SItemType::Sword:
            value = item->getEnchantValue(Enchant::SHARPNESS) + item->getItem()->getItemTier();
            break;

        case SItemType::Pickaxe:
        case SItemType::Axe:
        case SItemType::Shovel:
            value = item->getEnchantValue(Enchant::EFFICIENCY) + item->getItem()->getItemTier();
            break;

        default:
            break;
    }

    return value;
}

int ItemUtils::getBestItem(SItemType type, bool hotbarOnly) {
    auto player = Game.getLocalPlayer();
    if(!player)
        return -1;
    auto supplies = player->getsupplies();
    auto container = supplies->getcontainer();

    int bestSlot = -1;
    int bestValue = -1;

    for(int i = 0; i < 36; i++) {
        if(hotbarOnly && i > 8)
            break;

        auto stack = container->getItem(i);
        if(!stack->mItem)
            continue;

        if(stack->getItem()->getItemType() != type)
            continue;

        int value = getItemValue(stack);
        if(value > bestValue) {
            bestValue = value;
            bestSlot = i;
        }
    }

    return bestSlot;
}
