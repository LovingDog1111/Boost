#include "BowSpam.h"
#include <Minecraft/InvUtil.h>
BowSpam::BowSpam() : Module("BowSpam", "Spam arrows quickly", Category::COMBAT) {
    registerSetting(new SliderSetting<int>("Delay", "NULL", &delay, 5, 3, 32));
}

void BowSpam::onNormalTick(LocalPlayer* localPlayer) {
    if(!GI::canUseMoveKeys())
        return;

    if(!GI::isRightClickDown())
        return;

    static int Odelay = 0;
    ItemStack* carriedItem = InvUtil::getItem(InvUtil::getSelectedSlot());
    if(InvUtil::isVaildItem(carriedItem) && carriedItem->mItem.get()->mItemId == 306) {
        if(localPlayer->getItemUseDuration() == 0) {
            localPlayer->gamemode->baseUseItem(carriedItem);
            Odelay = 0;
        } else {
            if(Odelay >= delay) {
                localPlayer->gamemode->releaseUsingItem();
                Odelay = 0;
            } else {
                Odelay++;
            }
        }
    } else {
        Odelay = 0;
    }
}