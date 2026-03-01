#include "Offhand.h"

#include "../../../../../../Utils/Logger.h"
#include <string>
#include "..\Utils\Minecraft\InvUtil.h"

Offhand::Offhand()
    : Module("Offhand", "Automatically puts totems/shields into your offhand", Category::PLAYER) {
    itemModeEnum = (EnumSetting*)registerSetting(
        new EnumSetting("Item", "Item to switch ", {"Totem", "Shield"}, &itemMode, 0));
}

std::string Offhand::getModeText() {
    return itemModeEnum->enumList[itemMode];
}

void Offhand::onNormalTick(LocalPlayer* localPlayer) {
    int desiredMode = itemMode;

    ItemStack* offhandStack = localPlayer->getOffhandSlot();
    if(offhandStack && offhandStack->mItem.get() != nullptr)
        return;

    uint16_t ItemID = (desiredMode == 0) ? 601 : 362;
 

    Container* inventory = localPlayer->getsupplies()->container;
    for(int i = 0; i < 36; i++) {
        ItemStack* itemStack = inventory->getItem(i);
        Item* item = itemStack->getItem();
        if(item != nullptr && item->mItemId == ItemID) {
            localPlayer->setOffhandSlot(itemStack);
            InvUtil::sendMobEquipment(i);  

            inventory->removeItem(i, 1);
            break;
        }
    }
}